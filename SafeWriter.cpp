#include "SafeWriter.h"

#include <boost/filesystem.hpp>

SafeWriter::SafeWriter(const boost::filesystem::path &file)
	:
	  fileName(file.string())
{
	if (boost::filesystem::exists(file)) {
		throw "Unable to write to file (file already exists): " + fileName;
	}

	os.open(file.string().c_str());
	if (! os.good()) {
		throw "File opening failed: " + fileName;
	}
}

void SafeWriter::writeLine(const std::string &str) {
	os << str << std::endl;

	if (! os.good()) {
		throw "Unable to write to file: " + fileName;
	}
}
