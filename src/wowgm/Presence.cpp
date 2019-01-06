#include "Presence.hpp"

#include <Game/Entities/ObjectMgr.hpp>
#include <Game/Entities/CGPlayer.hpp>

#include <thread>
#include <mutex>
#include <sstream>
#include <random>

#include <discord/discord_rpc.h>

#include <shared/log/log.hpp>

#include <cstdlib>

namespace wowgm
{
    namespace discord
    {
        static std::once_flag initializer_flag;
        static time_t _startTime;

        void terminateDiscord();

        void initialize()
        {
#ifdef DISCORD_PRESENCE
            std::call_once(initializer_flag, []() -> void {
                DiscordEventHandlers handlers{};
                // handlers.ready = &handleDiscordReady;
                // handlers.errored = &handleDiscordError;
                // handlers.disconnected = &handleDiscordDisconnected;
                // handlers.joinGame = &handleDiscordJoinGame;
                // handlers.spectateGame = &handleDiscordSpectateGame;
                // handlers.joinRequest = &handleDiscordJoinRequest;

                _startTime = time(nullptr);

                Discord_Initialize("529792165870567454", &handlers, 1, nullptr);

                std::atexit(&terminateDiscord);
            });
#endif
        }

        void updatePresence()
        {
#ifdef DISCORD_PRESENCE
            initialize();

            using namespace wowgm::game::entities;

            const char* assets[] = {
                "pepodrink",
                // "erynstarin",
            };

            std::random_device rd;  //Will be used to obtain a seed for the random number engine
            std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
            std::uniform_int_distribution<> dis(0, sizeof(assets) / sizeof(const char*) - 1);

            DiscordRichPresence drp{};
            drp.state = nullptr;
            drp.startTimestamp = _startTime;
            drp.largeImageKey = assets[dis(gen)];

            LOG_INFO("DISCORD: {}", drp.largeImageKey);

            CGPlayer* localPlayer = ObjectAccessor::GetLocalPlayer();
            if (localPlayer == nullptr)
                drp.details = "Not connected.";
            else {
                std::stringstream ss;
                ss << "Logged in as {}";
                drp.details = ss.str().c_str();
            }

            Discord_UpdatePresence(&drp);
#endif
        }

        void terminateDiscord()
        {
            Discord_Shutdown();
        }
    }
}