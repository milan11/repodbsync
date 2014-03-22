#include "Outs.h"

#include <boost/filesystem.hpp>
#include "util.h"

Outs::Outs(boost::filesystem::path dir)
	:
	  dir(std::move(dir))
{
	if (boost::filesystem::exists(dir) && (! boost::filesystem::is_empty(dir))) {
		throw std::string("Outs directory is not empty. Maybe there are some unresolved filed from previous run.");
	}

	if (boost::filesystem::exists(dir) && (! boost::filesystem::is_directory(dir))) {
		throw std::string("Outs file exists but is not a directory.");
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
		throw std::string("File already exists: ") + file.string();
	}

	return file;
}
