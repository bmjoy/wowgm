#include "Utils.hpp"

#include <mutex>
#include <algorithm>
#include <stdarg.h>

#ifdef WIN32
#include <Windows.h>
#endif
#include <iostream>

#include <boost/filesystem.hpp>

const std::string& vkx::getAssetPath()
{
    static std::string path;
    static std::once_flag once;
    std::call_once(once, [] {
        auto current_path = boost::filesystem::current_path();
        current_path /= "resources/";
        path = current_path.string();
    });
    return path;
}
