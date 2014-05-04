#pragma once

#include <string>
#include <boost/format.hpp>

std::string stripDirectories(const std::string &path);

#define THROW(text) throw std::runtime_error((boost::format(text).str() + " [" + stripDirectories(__FILE__) + " " + std::to_string(__LINE__) + "]").c_str())

#define PRINT(exception) std::cerr << exception.what() << std::endl;

#define HANDLE_RETHROW(text) catch (const std::runtime_error &e) { PRINT(e); THROW(text); }
#define HANDLE_PRINT_AND_RETURN catch (const std::runtime_error &e) { PRINT(e); return 1; }
#define HANDLE_IGNORE catch (const std::runtime_error &e) {}
