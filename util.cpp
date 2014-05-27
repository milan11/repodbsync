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

		unsigned long result_ul = std::stoul(str, &pos);
		if (pos != str.size()) {
			throw 0;
		}

		if (result_ul > static_cast<unsigned long>(std::numeric_limits<uint32_t>::max())) {
			throw 1;
		}

		result = static_cast<uint32_t>(result_ul);
	} catch (...) {
		THROW(boost::format("Not a valid number: %1%") % str);
	}

	return result;
}
