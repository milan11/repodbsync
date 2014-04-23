#include "util.h"

#include <boost/filesystem.hpp>
#include "exceptions.h"

void createDirIfNotExists(const boost::filesystem::path &dir) {
	if (! boost::filesystem::exists(dir)) {
		try {
			boost::filesystem::create_directory(dir);
		} HANDLE_RETHROW(boost::format("Unable to create directory: %1%") % dir.string());
	}
}

void executeCommand(const std::string &command) {
	if (::system(command.c_str()) != 0) {
		THROW(boost::format("command failed: %1%") % command);
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

uint32_t stringToNumber(const std::string &str) {
	uint32_t result = 0;
	try {
		size_t pos;
		result = std::stoul(str, &pos);
		if (pos != str.size()) {
			throw 0;
		}
	} catch (...) {
		THROW(boost::format("Not a number: %1%") % str);
	}

	return result;
}
