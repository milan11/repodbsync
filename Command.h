#pragma once

#include <map>
#include <sstream>
#include <string>
#include <boost/filesystem/path.hpp>

class Command {

public:
	Command(const std::string &command);
	Command &appendArgument(const std::string &arg);
	Command &appendRedirectTo(const boost::filesystem::path &path);
	Command &appendRedirectFrom(const boost::filesystem::path &path);
	Command &setVariable(const std::string &name, const std::string &value);
	void execute();

private:
	std::string buildCommand() const;
	static std::string quotePart(const std::string &orig);

private:
	std::ostringstream ss;
	using VariablesMap = std::map<std::string, std::string>;
	VariablesMap variables;

};
