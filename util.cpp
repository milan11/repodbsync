#include "util.h"

#include <boost/filesystem.hpp>

void createDirIfNotExists(const boost::filesystem::path &dir) {
	if (! boost::filesystem::exists(dir)) {
		try {
			boost::filesystem::create_directory(dir);
		} catch (boost::filesystem::filesystem_error &e) {
			throw "Unable to create directory: " + dir.string() + " (cause: " + e.what() + ")";
		}
	}
}

void executeCommand(const std::string &command) {
	if (::system(command.c_str()) != 0) {
		throw std::string("command failed: ") + command;
	}
}

std::string toAlignedString(const uint32_t number, const size_t alignTo) {
	std::string numberString = std::to_string(number);
	if (numberString.size() < alignTo) {
		return std::string(alignTo - numberString.size(), '0') + numberString;
	} else {
		return numberString;
	}
}
