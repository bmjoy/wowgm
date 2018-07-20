#if 0

#include <boost/program_options.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>

#include <boost/stacktrace.hpp>

#include <iostream>
#include <iomanip>
#include <thread>

#include "AuthSocket.hpp"
#include "Updater.hpp"
#include "SocketManager.hpp"
#include "SignalHandler.hpp"
#include "Logger.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace po = boost::program_options;
namespace ip = boost::asio::ip;

static boost::asio::io_context context;

using namespace wowgm::networking;
using namespace wowgm::networking::authentification;
using namespace wowgm::threading;

int main(int argc, char* argv[])
{
    wowgm::log::initialize();

    try
    {
        std::shared_ptr<Updater> updater = std::make_shared<Updater>();

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "Print this help message.")
            ("server,s", po::value<std::string>()->default_value("127.0.0.1"), "The address of the server to connect to.")
            ("accountname,a", po::value<std::string>()->required(), "Account name to log in with.")
            ("password,p", po::value<std::string>()->required(), "The password associated to the account.");

        po::variables_map mapped_values;
        po::store(po::parse_command_line(argc, argv, desc), mapped_values);

        if (mapped_values.count("help") != 0 || mapped_values.empty())
        {
            std::cout << desc << std::endl;
            return 0;
        }

        bool anyMissing = false;
        for (auto&& descKey : desc.options())
        {
            if (!descKey->semantic()->is_required())
                continue;

            if (!mapped_values.count(descKey->long_name()) == 0)
                continue;

            std::cout << "Missing mandatory " << descKey->format_name() << " option.\n";
            anyMissing = true;
        }

        if (anyMissing)
        {
            std::cout << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }

        std::cout << std::endl;
        std::cout << "`7MMF'     A     `7MF'                              .g8\"\"\"bgd  `7MMM.     ,MMF'" << std::endl;
        std::cout << "  `MA     ,MA     ,V                              .dP'     `M    MMMb    dPMM" << std::endl;
        std::cout << "   VM:   ,VVM:   ,V    ,pW\"Wq.  `7M'    ,A    `MF'dM'       `    M YM   ,M MM" << std::endl;
        std::cout << "    MM.  M' MM.  M'   6W'   `Wb   VA   ,VAA   ,V  MM             M  Mb  M' MM" << std::endl;
        std::cout << "    `MM A'  `MM A'    8M     M8    VA ,V  VA ,V   MM.    `7MMF'  M  YM.P'  MM" << std::endl;
        std::cout << "     :MM;    :MM;     YA.   ,A9     VVV    VVV    `Mb.     MM    M  `YM'   MM" << std::endl;
        std::cout << "      VF      VF       `Ybmd9'       W      W       `\"bmmmdPY  .JML. `'  .JMML." << std::endl;
        std::cout << std::endl;

        auto authserver = mapped_values["server"].as<std::string>();
        auto username = mapped_values["accountname"].as<std::string>();
        auto password = mapped_values["password"].as<std::string>();

        LOG_INFO << "Connecting to grunt server on " << mapped_values["server"].as<std::string>() << ":3724 as " << username << ":" << password << "." << std::endl;

        ip::tcp::endpoint authEndpoint(ip::make_address(authserver), 3724);

        {
            std::shared_ptr<SocketManager> socketUpdater(updater->CreateUpdatable<SocketManager>());
            std::shared_ptr<AuthSocket> loginSocket = socketUpdater->Create<AuthSocket>(socketUpdater->GetContext());
            loginSocket->Connect(authEndpoint);

            loginSocket->SendAuthChallenge(username, password);
        }

        updater->Start();

        std::uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        LOG_GRAPHICS << extensionCount << " extensions found." << std::endl;

        updater->Stop();
    }
    catch (const std::exception& e)
    {
        std::cout << "[Exception] " << e.what() << '.' << std::endl;
        std::cout << boost::stacktrace::stacktrace() << std::endl;
    }

    std::cout << std::endl;
    return 0;
}

#else
#include "Window.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "LogicalDevice.hpp"
#include "SwapChain.hpp"
#include "PhysicalDevice.hpp"
#include "Assert.hpp"
#include "Shader.hpp"
#include "Pipeline.hpp"
#include "FrameBuffer.hpp"
#include "Queue.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Command.hpp"
#include "RenderPass.hpp"
#include "FrameBuffer.hpp"

#include <iostream>
#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>
#include <vulkan/vulkan.h>

int main()
{
    using namespace wowgm::graphics;

    try {
        Window* window = new Window(800, 600, "Vulkan");
        window->InitializeWindow();

        auto instance = Instance::Create("Vulkan", "No Engine");
        instance->SetupDebugCallback();

        Surface* surface = instance->CreateSurface(window); // Owned by instance
        LogicalDevice* device = instance->CreateLogicalDevice(); // Owned by instance
        SwapChain* swapChain = new SwapChain(instance->GetPhysicalDevice());

        // std::cout << instance->ToString();

        RenderPass* renderPass = new RenderPass(device);
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChain->GetSurfaceFormat().format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        renderPass->AddAttachment(colorAttachment);
        renderPass->Finalize();

        Pipeline* pipeline = new Pipeline(swapChain, renderPass);

        Shader* vertexShader = Shader::CreateVertexShader(device, "main", "C:\\Users\\Vincent Piquet\\source\\repos\\WowGM\\src\\wowgm\\Shaders\\vert.spv");
        Shader* fragmentShader = Shader::CreateFragmentShader(device, "main", "C:\\Users\\Vincent Piquet\\source\\repos\\WowGM\\src\\wowgm\\Shaders\\frag.spv");

        pipeline->SetDepthTest(false);
        pipeline->SetStencilTest(false);
        pipeline->SetWireframe(false);
        pipeline->SetPrimitiveType(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        pipeline->SetCulling(VK_CULL_MODE_NONE);
        pipeline->AddShader(vertexShader);
        pipeline->AddShader(fragmentShader);
        pipeline->Finalize();

        std::vector<FrameBuffer*> buffers;

        for (int i = 0; i < 3; ++i)
        {
            FrameBuffer* frameBuffers = new FrameBuffer(renderPass, swapChain); // leak
            frameBuffers->AttachImageView(swapChain->GetImageView(i));
            frameBuffers->Finalize();
            buffers.push_back(frameBuffers);

            CommandBuffer* drawBuffer = device->GetGraphicsQueue()->GetCommandPool()->AllocatePrimaryBuffer();
            drawBuffer->BeginRecording(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
            drawBuffer->Record<BeginRenderPass>(frameBuffers, swapChain->GetExtent());
            drawBuffer->Record<BindPipeline>(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            drawBuffer->Draw(3);
            drawBuffer->Record<EndRenderPass>();
            drawBuffer->FinishRecording();
            device->AddCommandBuffer(drawBuffer);
        }

        while (!window->ShouldClose())
        {
            window->Execute();
            device->Draw(swapChain);
        }

        device->WaitIdle();

        delete pipeline;
        delete swapChain;
        instance.reset();

        window->Cleanup();
        delete window;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;

        const boost::stacktrace::stacktrace* st = boost::get_error_info<traced>(e);
        if (st)
            std::cerr << *st << std::endl;
    }
    return 0;
}

#endif
