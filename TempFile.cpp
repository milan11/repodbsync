#include "TempFile.h"

#include <boost/filesystem.hpp>

TempFile::TempFile(boost::filesystem::path file)
	:
	  file(std::move(file)),
	  moved(false)
{
}

TempFile::~TempFile() {
	if (! moved) {
		try {
			boost::filesystem::remove(file);
		} catch (...) {
		}
	}
}

const boost::filesystem::path &TempFile::path() const {
	return file;
}

void TempFile::moveTo(const boost::filesystem::path &newPath) {
	if (moved) {
		throw "Temp file already moved: " + file.string();
	}

	try {
		boost::filesystem::rename(file, newPath);
	} catch (boost::filesystem::filesystem_error &e) {
		throw "Unable to move temporary file: " + file.string() + " (cause: " + e.what() + ")";
	}

	moved = true;
}
