#include "SafeWriter.h"

#include <boost/filesystem.hpp>
#include "exceptions.h"

SafeWriter::SafeWriter(const boost::filesystem::path &file)
	:
	  fileName(file.string()),
	  closed(false)
{
	if (boost::filesystem::exists(file)) {
		THROW(boost::format("Unable to write to file (file already exists): %1%") % fileName);
	}

	os.open(file.string().c_str());
	if (! os.good()) {
		THROW(boost::format("File opening failed: %1%") % fileName);
	}
}

SafeWriter::~SafeWriter() {
	if (closed) {
		return;
	}

	try {
		close();
	} catch (...) {
	}
}

void SafeWriter::writeLine(const std::string &str) {
	os << str << std::endl;

	if (! os.good()) {
		THROW(boost::format("Unable to write to file: %1%") % fileName);
	}
}

void SafeWriter::close() {
	closed = true;
	os.close();
	if (! os.good()) {
		THROW(boost::format("Unable to close file: %1%") % fileName);
	}
}
