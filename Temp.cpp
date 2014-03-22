#include "Temp.h"

#include <boost/filesystem.hpp>
#include "util.h"

Temp::Temp(boost::filesystem::path dir)
	:
	  dir(std::move(dir))
{
	if (boost::filesystem::exists(dir) && (! boost::filesystem::is_directory(dir))) {
		throw std::string("Temp file exists but is not a directory.");
	}
}

Temp::~Temp() {
	try {
		if (boost::filesystem::is_directory(dir) && boost::filesystem::is_empty(dir)) {
			boost::filesystem::remove(dir);
		}
	} catch (...) {
	}
}

TempFile Temp::createFile() {
	createDirIfNotExists(dir);

	return TempFile(generateRandomChildPath());
}

const boost::filesystem::path &Temp::path() const {
	return dir;
}

boost::filesystem::path Temp::generateRandomChildPath() {
	::srand(::time(NULL));

	while (true) {
		int num = ::rand();
		if (assignedNumbers.find(num) == assignedNumbers.end()) {
			assignedNumbers.insert(num);
			return dir / (std::string("tmpfile_") + std::to_string(num).c_str());
		}
	}
}
