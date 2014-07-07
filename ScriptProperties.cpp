#include "ScriptProperties.h"

#include "exceptions.h"
#include "util.h"

ScriptProperties::ScriptProperties(const boost::filesystem::path &file) {
	if (file.extension() != ".sql") {
		THROW(boost::format("Invalid file extension (should be .sql): %1%") % file.string());
	}

	const std::string stem = file.stem().string();

	const std::string targetDbVersionNum = stem.substr(0, versionLength);

	try {
		targetDbVersion = ::stringToNumber(targetDbVersionNum);
	} HANDLE_RETHROW(boost::format("Script file name should begin with a number (%1% digits): %2%") % size_t(versionLength) % file.string());

	if (stem.size() < (versionLength + 1) || (stem[versionLength] != '_')) {
		THROW(boost::format("Number in a script file name has to begin with an underscore (_): %1%") % file.string());
	}

	name = stem.substr(versionLength + 1);
}

ScriptProperties::ScriptProperties(const uint32_t targetDbVersion, std::string name)
:
	targetDbVersion(targetDbVersion),
	name(std::move(name))
{
}

uint32_t ScriptProperties::getTargetDbVersion() const {
	return targetDbVersion;
}

const std::string &ScriptProperties::getName() const {
	return name;
}

std::string ScriptProperties::toFileName() const {
	return ::toAlignedString(targetDbVersion, versionLength) + '_' + name + ".sql";
}
