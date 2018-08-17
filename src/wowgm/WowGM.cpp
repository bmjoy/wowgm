#if 0
#include <cstring>
#include <iostream>
#include <iomanip>
#include <cstdint>

int main()
{
    std::uint8_t N_[] = {
        0x80, 0x60, 0x57, 0xd3, 0xe5, 0x0d, 0x9d, 0x99, 0x95, 0x26, 0x27, 0x24,
        0x64, 0xc7, 0x94, 0x77, 0x08, 0x29, 0x65, 0x13
    };
    std::uint8_t g_[] = {
        0x5d, 0x1b, 0xe7, 0xe9, 0xdd, 0xa1, 0xee, 0x88, 0x96, 0xbe, 0x5b, 0x7e,
        0x34, 0xa8, 0x5e, 0xe1, 0x64, 0x52, 0xa7, 0xb4
    };

    std::uint8_t gN[20];
    memcpy(gN, N_, sizeof(N_));
    for (std::uint8_t i = 0; i < sizeof(N_); ++i)
        gN[i] ^= g_[i];

    std::uint8_t output[] = {
        0xdd, 0x7b, 0xb0, 0x3a, 0x38, 0xac, 0x73, 0x11, 0x03, 0x98, 0x7c, 0x5a,
        0x50, 0x6f, 0xca, 0x96, 0x6c, 0x7b, 0xc2, 0xa7
    };

    for (std::uint8_t i = 0; i < sizeof(gN); ++i)
        if (output[i] != gN[i])
            std::cout << "Mismatch: gN[" << std::dec << i << "] = 0x" << std::hex << std::setw(2) << std::setfill('0') << gN[i]
                      << " output[" << std::dec << i << "] = 0x" << std::hex << std::setw(2) << std::setfill('0') << output[i];

    return 0;
}

#else
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

        wowgm::Window window("WowGM");
        window.Run();
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
#endif