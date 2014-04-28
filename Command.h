#pragma once

#include <sstream>
#include <string>
#include <boost/filesystem/path.hpp>

class Command {

public:
	Command(const std::string &command);
	Command &appendArgument(const std::string &arg);
	Command &appendRedirectTo(const boost::filesystem::path &path);
	Command &appendRedirectFrom(const boost::filesystem::path &path);
	void execute();

private:
	std::string quotePart(const std::string &orig);

private:
	std::ostringstream ss;

};
