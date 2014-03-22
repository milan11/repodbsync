#pragma once

#include <set>
#include <boost/filesystem/path.hpp>
#include "TempFile.h"

class Temp {
public:
	explicit Temp(boost::filesystem::path dir);
	~Temp();

	TempFile createFile();
	const boost::filesystem::path &path() const;

private:
	boost::filesystem::path generateRandomChildPath();

private:
	boost::filesystem::path dir;
	std::set<int> assignedNumbers;

};
