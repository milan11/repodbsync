#include "TempFile.h"

#include <boost/filesystem.hpp>
#include "exceptions.h"

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
		THROW(boost::format("Temp file already moved: %1%") % file.string());
	}

	try {
		boost::filesystem::rename(file, newPath);
	} HANDLE_RETHROW(boost::format("Unable to move temporary file: %1%") % file.string());

	moved = true;
}
