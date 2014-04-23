#include "LinesReader.h"
#include "exceptions.h"

LinesReader::LinesReader(const boost::filesystem::path &file)
	:
	  is(file.string().c_str()),
	  fileName(file.string())
{
	if (! is.good()) {
		THROW(boost::format("File opening failed: %1%") % fileName);
	}
}

boost::optional<std::string> LinesReader::readLine() {
	if (is.eof()) {
		return boost::optional<std::string>();
	}

	std::string line;
	std::getline(is, line);
	if ((! is.good()) && (! is.eof())) {
		THROW(boost::format("File reading failed: %1%") % fileName);
	}

	if (is.eof() && line.empty()) {
		return boost::optional<std::string>();
	}

	return line;
}
