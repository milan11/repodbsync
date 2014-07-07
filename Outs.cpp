#include "Outs.h"

#include <boost/filesystem.hpp>
#include "exceptions.h"
#include "util.h"

Outs::Outs(boost::filesystem::path dir)
:
	dir(std::move(dir))
{
	if (boost::filesystem::exists(dir) && (! boost::filesystem::is_empty(dir))) {
		THROW("Outs directory is not empty. Maybe there are some unresolved filed from previous run.");
	}

	if (boost::filesystem::exists(dir) && (! boost::filesystem::is_directory(dir))) {
		THROW("Outs file exists but is not a directory.");
	}
}

Outs::~Outs() {
	try {
		if (boost::filesystem::is_directory(dir) && boost::filesystem::is_empty(dir)) {
			boost::filesystem::remove(dir);
		}
	} catch (...) {
	}
}

boost::filesystem::path Outs::createFile(const std::string &name) {
	createDirIfNotExists(dir);

	boost::filesystem::path file = dir / name;
	if (boost::filesystem::exists(file)) {
		THROW(boost::format("File already exists: ") % file.string());
	}

	return file;
}
