#pragma once

#include <fstream>
#include <string>
#include <boost/filesystem/path.hpp>

class SafeWriter {
public:
	explicit SafeWriter(const boost::filesystem::path &file);
	~SafeWriter();

	void writeLine(const std::string &str);

	void close();

private:
	std::ofstream os;
	std::string fileName;
	bool closed;

};
