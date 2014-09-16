#pragma once

#include <map>
#include <set>
#include <string>

enum class ManualItem {
	SETTINGS_FILE,
	DATABASE_VERSIONING,
	APPLYING_SCRIPTS,
	ADDING_SCRIPTS_TO_REPOSITORY,
};

struct ManualItemHeading {
	std::string number;
	std::string title;
};

class Manual {

public:
	Manual();

public:
	std::set<ManualItem> getAll() const;
	const ManualItemHeading &getHeading(const ManualItem item) const;

private:
	using ItemsMap = std::map<ManualItem, ManualItemHeading>;
	ItemsMap items;

};

std::string seeManual(const ManualItem item);
