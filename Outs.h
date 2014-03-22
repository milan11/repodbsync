#pragma once

#include <boost/filesystem/path.hpp>

class Outs {
public:
	explicit Outs(boost::filesystem::path dir);
	~Outs();

	boost::filesystem::path createFile(const std::string &name);

private:
	const boost::filesystem::path dir;

};
