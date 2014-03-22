#pragma once

#include <boost/filesystem/path.hpp>

class TextDiff {
public:
	TextDiff(const boost::filesystem::path &a, const boost::filesystem::path &b);

public:
	bool areEqual() const;

private:
	bool equal;
};
