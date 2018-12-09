#include <boost/program_options.hpp>
#include <boost/stacktrace/stacktrace.hpp>

#include <iostream>
#include <string>
#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Logger.hpp"
#include "Updater.hpp"
#include "BigNumber.hpp"
#include "FileSystem.hpp"

#include "glfw.hpp"

#include "Window.hpp"
#include "GameWindow.hpp"

#include <boost/exception/get_error_info.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    // Seed the OpenSSL's PRNG here.
    // That way it won't auto-seed when calling BigNumber::SetRand and slow down the first world login
    wowgm::cryptography::BigNumber seed;
    seed.SetRand(16 * 8);

    wowgm::log::initialize();

    DiskFileSystem::Instance()->Initialize(".");

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "Print this help message.")
            ("server,s", po::value<std::string>()->default_value("127.0.0.1"), "The address of the server to connect to.");

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
        sUpdater->Start();
#if 0
        wowgm::Window window("WowGM");
        window.Run();
#else
        GameWindow window("WowGM");
        window.InitializeWindow(false);
        window.InitializeVulkan();
        window.InitializeInterface();

        window.Run();

        window.PrepareForRelease();
#endif

        sUpdater->Stop();
    }
    catch (const boost::system::system_error& se)
    {
        std::cerr << se.code() << " " << se.what() << std::endl;

        const boost::stacktrace::application_stacktrace* st = boost::get_error_info<traced>(se);
        if (st)
            std::cerr << *st << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;

        const boost::stacktrace::application_stacktrace* st = boost::get_error_info<traced>(e);
        if (st)
            std::cerr << *st << std::endl;
    }
    return 0;
}
