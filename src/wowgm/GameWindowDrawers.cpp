#include "GameWindow.hpp"
#include "FileSystem.hpp"
#include "ClientServices.hpp"
#include "AuthResult.hpp"

#include <imgui.h>
#include <cstdint>
#include <string>
#include <future>
#include <numeric>

#ifdef min
#undef min
#undef max
#endif

namespace gui::gamedata
{
    char gameInstallationPath[MAX_PATH] { };
    char realmlist[0x100] { };
    char accountName[100] { };
    char accountPassword[100] { };
}

void GameWindow::DrawLoadingScreen()
{
    using namespace wowgm::filesystem;

    ImGui::NewFrame();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ float(_width), float(_height) });

    /// Title logo
    ImGui::Begin("##WelcomeScreen", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    constexpr static const char APPLICATION_NAME[] = "WoWGM";

    uint32_t positionX = 0;
    uint32_t positionY = 10;
    uint32_t contentWidth = 0;

    ImGui::PushFont(io.Fonts->Fonts[4]); // Ubuntu-Bold.TTF
    auto applicationLogoSize = ImGui::CalcTextSize(APPLICATION_NAME);
    positionX = (_width - applicationLogoSize.x) / 2;

    ImGui::SetCursorPosX(positionX);
    ImGui::SetCursorPosY(positionY);
    contentWidth = applicationLogoSize.x;
    ImGui::Text(APPLICATION_NAME);
    ImGui::PopFont();

    positionY += applicationLogoSize.y;

#if _DEBUG
    /// Debug build notice
# if PLATFORM == PLATFORM_WINDOWS
    constexpr static const char APPLICATION_DEBUG_SUBTITLE[] = "DEVELOPMENT BUILD - WINDOWS - FOR DEBUGGING ONLY";
# elif PLATFORM == PLATFORM_UNIX
    constexpr static const char APPLICATION_DEBUG_SUBTITLE[] = "DEVELOPMENT BUILD - UNIX - FOR DEBUGGING ONLY";
# elif PLATFORM == PLATFORM_APPLE
    constexpr static const char APPLICATION_DEBUG_SUBTITLE[] = "DEVELOPMENT BUILD - MACOS - FOR DEBUGGING ONLY";
# else
    constexpr static const char APPLICATION_DEBUG_SUBTITLE[] = "DEVELOPMENT BUILD - FOR DEBUGGING ONLY";
# endif
    positionY += 5; // Add a 5-pixels wide margin beneath the title so that stuff doesn't glue together.
    ImGui::PushFont(io.Fonts->Fonts[5]); // 04B-03__.TTF
    auto debugLabelSize = ImGui::CalcTextSize(APPLICATION_DEBUG_SUBTITLE);
    ImGui::SetCursorPosX((_width - debugLabelSize.x) / 2);
    ImGui::SetCursorPosY(positionY);
    ImGui::Text(APPLICATION_DEBUG_SUBTITLE);
    ImGui::PopFont();

    positionY += debugLabelSize.y;
#endif

    // Add a relatively large margin so people know what's up
    positionY += 25;

    /// Game installation path title, and text input
    ImGui::SetCursorPosX(positionX);
    ImGui::SetCursorPosY(positionY);
    ImGui::PushFont(io.Fonts->Fonts[2]); // Actual text font, with proper size for sections.
    ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, "Path to Wow.exe"); // Yellow
    ImGui::PopFont();
    ImGui::PushItemWidth(contentWidth);
    ImGui::SetCursorPosX(positionX);
    ImGui::InputText("##GameDataFolder", gui::gamedata::gameInstallationPath, sizeof(gui::gamedata::gameInstallationPath), ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiTextEditCallbackData* callbackData) -> int {
#if PLATFORM == PLATFORM_WINDOWS
        switch (callbackData->EventChar)
        {
        case '<':
        case '>':
        case '"':
        case '\\':
        case '|':
        case '?':
        case '*':
            return 1;
        }
#endif
        return 0;
    });

    // Show an extra line if we found game data
    bool gameBinaryFound = DiskFileSystem::Instance()->FileExists("./Wow.exe", gui::gamedata::gameInstallationPath); // FIXME provide user specified location
    ImGui::SetCursorPosX(positionX);
    ImGui::PushFont(io.Fonts->Fonts[1]); // Regular font, but smaller size.
    ImGui::TextColored({
        !gameBinaryFound ? 1.0f : 0.0f,
        !gameBinaryFound ? 0.0f : 1.0f,
        0.0f,
        1.0f
    }, gameBinaryFound ? "Game data loaded." : "Incorrect directory specified.");
    ImGui::PopFont();
    positionY += 95; // Margin again

    if (gameBinaryFound)
    {
        std::thread([&]() -> void {
            MpqFileSystem::Instance()->Initialize(gui::gamedata::gameInstallationPath);
            _mpqSystemInitialized.exchange(true);
        }).detach();
    }

    // Grunt-specific data (realmname, port, accname, accpwd)
    ImGui::SetCursorPosX(positionX);
    ImGui::SetCursorPosY(positionY);
    ImGui::PushFont(io.Fonts->Fonts[2]); // Actual text font, with proper size for sections.
    ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, "Server information"); // Yellow
    ImGui::PopFont();

    auto inputTextLabelSize = ImGui::CalcTextSize("Realm server address");
    ImGui::SetCursorPosX(positionX);
    ImGui::PushItemWidth(contentWidth - inputTextLabelSize.x - ImGui::GetStyle().ItemInnerSpacing.x * 2);
    ImGui::InputText("Realm server address", gui::gamedata::realmlist, sizeof(gui::gamedata::realmlist), gameBinaryFound ? 0 : ImGuiInputTextFlags_ReadOnly);

    ImGui::SetCursorPosX(positionX);
    ImGui::PushItemWidth(contentWidth - inputTextLabelSize.x - ImGui::GetStyle().ItemInnerSpacing.x * 2);
    ImGui::InputText("Account name##AccountName", gui::gamedata::accountName, sizeof(gui::gamedata::accountName), gameBinaryFound ? 0 : ImGuiInputTextFlags_ReadOnly);

    ImGui::SetCursorPosX(positionX);
    ImGui::PushItemWidth(contentWidth - inputTextLabelSize.x - ImGui::GetStyle().ItemInnerSpacing.x * 2);
    ImGui::InputText("Password##AccountPassword", gui::gamedata::accountPassword, sizeof(gui::gamedata::accountPassword), ImGuiInputTextFlags_Password | (gameBinaryFound ? 0 : ImGuiInputTextFlags_ReadOnly));

    positionY += 140;
    auto submitButtonSize = ImGui::CalcTextSize("Connect");
    ImGui::SetCursorPosX((_width - submitButtonSize.x) / 2);
    ImGui::SetCursorPosY(positionY);

    if (!gameBinaryFound || !_mpqSystemInitialized)
        ImGui::PushDisabled();

    if (ImGui::Button("Connect##RealmConnectButton"))
    {
        // async connect, and then display connect feedback
        uint32_t port = 3724;
        std::string_view realmList = gui::gamedata::realmlist;
        const char* portDelimiterPos = strchr(realmList.data(), ':');
        if (portDelimiterPos != nullptr)
        {
            port = std::atoi(portDelimiterPos + 1);
            realmList = realmList.substr(0, portDelimiterPos - gui::gamedata::realmlist);
        }

        sClientServices->AsyncConnect(gui::gamedata::accountName, gui::gamedata::accountPassword, realmList, port);
    }

    if (!gameBinaryFound || !_mpqSystemInitialized)
        ImGui::PopDisabled();

    using namespace wowgm::protocol::authentification;
    std::string authentificationResultString;
    switch (sClientServices->GetAuthentificationResult())
    {
        case LOGIN_OK:
        case LOGIN_NOT_STARTED:
            break;
        case LOGIN_BANNED:
            authentificationResultString = "This account has been banned.";
            break;
        case LOGIN_UNKNOWN_ACCOUNT:
            authentificationResultString = "Unknown account.";
            break;
        case LOGIN_BAD_VERSION:
            authentificationResultString = "Version mismatch.";
            break;
        case LOGIN_INVALID_SRP6:
            authentificationResultString = "Cryptography proof mismatch.";
            break;
        default:
            authentificationResultString = "Authentification result = ";
            authentificationResultString += std::to_string(sClientServices->GetAuthentificationResult());
            break;
    }

    std::string authentificationStepString;
    switch (sClientServices->GetAuthentificationStatus())
    {
        case AUTH_LOGON_CHALLENGE:
            authentificationStepString = "Connecting ...";
            break;
        case AUTH_LOGON_PROOF:
            authentificationStepString = "Identificating ...";
            break;
        case REALM_LIST:
            authentificationStepString = "Retrieving realm list ...";
            break;
    }

    positionY += 50;

    if (!authentificationStepString.empty())
    {
        ImGui::PushFont(io.Fonts->Fonts[0]); // Regular font, but smaller size.
        ImGui::SetCursorPosX(positionX);
        ImGui::SetCursorPosY(positionY);
        ImGui::Text(authentificationStepString.c_str());
        ImGui::PopFont();

        positionY += 15;
    }

    if (!authentificationResultString.empty())
    {
        ImGui::PushFont(io.Fonts->Fonts[0]); // Regular font, but smaller size.
        ImGui::SetCursorPosX(positionX);
        ImGui::SetCursorPosY(positionY);
        ImGui::Text(authentificationResultString.c_str());
        ImGui::PopFont();

        positionY += 15;
    }

    positionY = _height - 115.0f;
    ImGui::SetCursorPosY(positionY);
    _DrawFrameTimeHistogram();

    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
}

void GameWindow::_DrawFrameTimeHistogram()
{
    // TODO: Dock this group to the bottom of the window it is contained into
    if (!_showFramesMetrics)
        return;

    ImGui::BeginGroup();
    float oldPosX = ImGui::GetCursorPosX();

    std::vector<float> histogram(_frameDrawTimes.begin(), _frameDrawTimes.end());
    std::vector<float>::iterator minScale;
    std::vector<float>::iterator maxScale;
    std::tie(minScale, maxScale) = std::minmax_element(histogram.begin(), histogram.end());

    constexpr static const char CHART_LABEL[] = "Frame Draw time (ms) %.3f - %.3f - Average FPS %.2f";
    auto labelTextSize = ImGui::CalcTextSize(CHART_LABEL);
    ImGui::SetCursorPosX(oldPosX + (_width - labelTextSize.x) / 2);
    ImGui::Text(CHART_LABEL, *minScale, *maxScale, 1000.0f * 1000.0f / std::accumulate(histogram.begin(), histogram.end(), 0.0f));

    ImGui::SetCursorPosX(oldPosX);

    ImGui::PushDisabled(false);
    ImGui::PlotLines("##FrameDrawTimeChart", histogram.data(), histogram.size(), 0, nullptr, *minScale, *maxScale, { float(_width) - oldPosX * 2, 75.0f });
    ImGui::PopDisabled(false);

    ImGui::EndGroup();

    _interfaceVertexCount = 0; // Force reupload vertices to properly upload the graph
}
