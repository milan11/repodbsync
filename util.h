#pragma once

#include <cstdint>
#include <string>
#include <boost/filesystem/path.hpp>

void createDirIfNotExists(const boost::filesystem::path &dir);
std::string toAlignedString(const uint32_t number, const size_t alignTo);
uint32_t stringToNumber(const std::string &str);
