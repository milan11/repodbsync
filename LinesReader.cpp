#include "LinesReader.h"

LinesReader::LinesReader(const boost::filesystem::path &file)
	:
	  is(file.string().c_str()),
	  fileName(file.string())
{
	if (! is.good()) {
		throw "File opening failed: " + fileName;
	}
}

boost::optional<std::string> LinesReader::readLine() {
	if (is.eof()) {
		return boost::optional<std::string>();
	}

	std::string line;
	std::getline(is, line);
	if ((! is.good()) && (! is.eof())) {
		throw "File reading failed: " + fileName;
	}

	if (is.eof() && line.empty()) {
		return boost::optional<std::string>();
	}

	return line;
}
