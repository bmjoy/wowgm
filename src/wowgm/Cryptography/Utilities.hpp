#pragma once

#include <cstdint>
#include <string>

std::string ByteArrayToHexStr(std::uint8_t const* bytes, std::uint32_t length, bool reverse = false);
