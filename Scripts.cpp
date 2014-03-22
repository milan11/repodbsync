#include "Scripts.h"

#include <boost/filesystem.hpp>
#include "ScriptProperties.h"
#include "util.h"

Scripts::Scripts(boost::filesystem::path dir)
	:
	  dir(std::move(dir))
{
	if (boost::filesystem::exists(dir) && (! boost::filesystem::is_directory(dir))) {
		throw std::string("Scripts file exists but is not a directory.");
	}

	createDirIfNotExists(dir);

	version = 0;
	boost::filesystem::directory_iterator end_iter;
	for (boost::filesystem::directory_iterator it(dir) ; it != end_iter ; ++it) {
		if (! boost::filesystem::is_regular_file(it->status())) {
			continue;
		}

		if (it->path().extension().string() == ".swp") {
			continue;
		}

		files.insert(it->path().string());
	}

	for (const boost::filesystem::path &file : files) {
		++version;

		ScriptProperties scriptProperties(file);

		if (scriptProperties.getTargetDbVersion() != version) {
			throw std::string("Missing script file with target database version ") + std::to_string(version) + " (" + std::to_string(scriptProperties.getTargetDbVersion()) + " found instead): " + file.string();
		}
	}
}

uint32_t Scripts::getVersion() const {
	return version;
}

const std::set<boost::filesystem::path> &Scripts::getFiles() const {
	return files;
}
