#pragma once

#include <cstdint>
#include <set>
#include <boost/filesystem/path.hpp>

class Scripts {
public:
	explicit Scripts(boost::filesystem::path dir);

	uint32_t getVersion() const;
	const std::set<boost::filesystem::path> &getFiles() const;

private:
	const boost::filesystem::path dir;
	uint32_t version;
	std::set<boost::filesystem::path> files;

};
