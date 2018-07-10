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

namespace po = boost::program_options;
namespace asio = boost::asio;
namespace ip = asio::ip;

static boost::asio::io_context context;

using namespace wowgm::networking;
using namespace wowgm::networking::authentification;
using namespace wowgm::threading;

int main(int argc, char* argv[])
{
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

        std::cout << "Connecting to grunt server on " << mapped_values["server"].as<std::string>() << ":3724 as " << username << ":" << password << "." << std::endl;

        ip::tcp::endpoint authEndpoint(ip::make_address(authserver), 3724);

        {
            std::shared_ptr<SocketManager> socketUpdater(updater->CreateUpdatable<SocketManager>());
            std::shared_ptr<AuthSocket> loginSocket = socketUpdater->Create<AuthSocket>(socketUpdater->GetContext());
            loginSocket->Connect(authEndpoint);

            loginSocket->SendAuthChallenge(username, password);
        }

        updater->Start();

        // Blocking loop until end of process
        wowgm::signal::install();
        while (true);

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

