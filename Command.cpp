#include "Command.h"

#include "exceptions.h"

Command::Command(const std::string &command) {
	ss << quotePart(command);
}

Command &Command::appendArgument(const std::string &arg) {
	ss << ' ';
	ss << quotePart(arg);

	return *this;
}

Command &Command::appendRedirectTo(const boost::filesystem::path &path) {
	ss << " > ";
	ss << quotePart(path.string());

	return *this;
}

Command &Command::appendRedirectFrom(const boost::filesystem::path &path) {
	ss << " < ";
	ss << quotePart(path.string());

	return *this;
}

void Command::execute() {
	const std::string command = ss.str();

	if (::system(command.c_str()) != 0) {
		THROW(boost::format("command failed: %1%") % command);
	}
}

std::string Command::quotePart(const std::string &orig) {
	const std::string quote = "\"";
	const std::string backslash = "\\";

	std::string escaped = orig;
	boost::algorithm::replace_all(escaped, backslash, backslash + backslash);
	boost::algorithm::replace_all(escaped, quote, backslash + quote);

	return quote + escaped + quote;
}
