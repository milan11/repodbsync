#include "exceptions.h"

std::string stripDirectories(const std::string &path) {
	std::string::size_type lastSlash = path.find_last_of("/\\");

	if (lastSlash != std::string::npos) {
		return path.substr(lastSlash + 1);
	} else {
		return path;
	}
}
