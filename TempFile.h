#pragma once

#include <boost/filesystem/path.hpp>

class TempFile {
public:
	explicit TempFile(boost::filesystem::path file);
	~TempFile();

	const boost::filesystem::path &path() const;
	void moveTo(const boost::filesystem::path &newPath);

private:
	const boost::filesystem::path file;
	bool moved;

};
