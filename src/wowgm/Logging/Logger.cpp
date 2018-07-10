#include "Logger.hpp"

const char* severity_level_str[] = {
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL",
    "DEBUG",
    "TRACE",
    "GRAPHICS"
};

namespace wowgm::log {

    void initialize()
    {
        logging::register_simple_formatter_factory<severity_level, char>("Severity");

        logging::add_common_attributes();

        logging::add_console_log(std::cout,
            keywords::format = "[%TimeStamp%] %Severity% - %Message%"
        );

    }

} // namespace wowgm::log
