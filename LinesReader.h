#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

class LinesReader {
public:
	explicit LinesReader(const boost::filesystem::path &file);

	boost::optional<std::string> readLine();

private:
	std::ifstream is;
	std::string fileName;
};
