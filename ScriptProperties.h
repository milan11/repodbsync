#pragma once

#include <cstdint>
#include <boost/filesystem/path.hpp>

class ScriptProperties {
public:
	explicit ScriptProperties(const boost::filesystem::path &file);
	ScriptProperties(const uint32_t targetDbVersion, std::string name);

	uint32_t getTargetDbVersion() const;
	const std::string &getName() const;

	std::string toFileName() const;

private:
	uint32_t targetDbVersion;
	std::string name;

	static const size_t versionLength = 6;

};
