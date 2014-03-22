#pragma once

#include <fstream>
#include <string>
#include <boost/filesystem/path.hpp>

class SafeWriter {
public:
	explicit SafeWriter(const boost::filesystem::path &file);

	void writeLine(const std::string &str);

private:
	std::ofstream os;
	std::string fileName;
};
