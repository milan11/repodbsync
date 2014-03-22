#include "ScriptProperties.h"

#include "util.h"

ScriptProperties::ScriptProperties(const boost::filesystem::path &file) {
	if (file.extension() != ".sql") {
		throw std::string("Invalid file extension (should be .sql):" + file.string());
	}

	const std::string stem = file.stem().string();

	const std::string targetDbVersionNum = stem.substr(0, versionLength);

	try {
		size_t pos;
		targetDbVersion = std::stoul(targetDbVersionNum, &pos);
		if (pos != targetDbVersionNum.size()) {
			throw 0;
		}
	} catch(...) {
		throw std::string("Script file name should begin with a number (" + std::to_string(versionLength) + " digits): " + file.string());
	}

	if (stem.size() < (versionLength + 1) || (stem[versionLength] != '_')) {
		throw std::string("Number in a script file name has to begin with an underscore (_): " + file.string());
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
