#pragma once

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/current_function.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;
namespace expr = boost::log::expressions;

// We define our own severity levels
enum severity_level
{
    e_info,
    e_warning,
    e_error,
    e_critical,
    e_debug,
    e_trace,
    e_rendering,
    e_performance,
    e_max_security,
};

extern const char* severity_level_str[];

template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT>& operator << (std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
{
    if (lvl < static_cast<int>(e_max_security))
        strm << severity_level_str[lvl];
    else
        strm << static_cast<int>(lvl);
    return strm;
}

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(severity_logger, src::severity_logger_mt<severity_level>)

namespace wowgm::log
{
    void initialize();

} // namespace wowgm::log

#define LOG_SCOPE       BOOST_LOG_NAMED_SCOPE(BOOST_CURRENT_FUNCTION);

#define LOG_INFO        BOOST_LOG_SEV(severity_logger::get(), e_info)
#define LOG_WARNING     BOOST_LOG_SEV(severity_logger::get(), e_warning)
#define LOG_ERROR       BOOST_LOG_SEV(severity_logger::get(), e_error)
#define LOG_CRITICAL    BOOST_LOG_SEV(severity_logger::get(), e_critical)

#define LOG_DEBUG       BOOST_LOG_SEV(severity_logger::get(), e_debug)
#if !_DEBUG
#pragma message("Consider #ifdef'ing LOG_DEBUG calls, as expensive operations may not be optimized out by the compiler.")
#endif

#define LOG_TRACE       BOOST_LOG_SEV(severity_logger::get(), e_trace)
#define LOG_RENDERING   BOOST_LOG_SEV(severity_logger::get(), e_rendering)
#define LOG_PERFORMANCE BOOST_LOG_SEV(severity_logger::get(), e_performance)
