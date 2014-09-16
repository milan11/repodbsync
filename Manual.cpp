#include "Manual.h"

#include <algorithm>
#include <sstream>

namespace {
	const std::string manualFileName = "README.md";
}

Manual::Manual()
:
	items{
		{ManualItem::SETTINGS_FILE, {"A.", "Settings file"}},
		{ManualItem::DATABASE_VERSIONING, {"B.", "Database versioning"}},
		{ManualItem::APPLYING_SCRIPTS, {"C.", "Applying scripts to local database"}},
		{ManualItem::ADDING_SCRIPTS_TO_REPOSITORY, {"D.", "Adding database scripts to the repository"}},
	}
{
}

std::set<ManualItem> Manual::getAll() const {
	std::set<ManualItem> result;

	for (const auto item : items) {
		result.insert(item.first);
	}

	return result;
}

const ManualItemHeading &Manual::getHeading(const ManualItem item) const {
	return items.at(item);
}

std::string seeManual(const ManualItem item) {
	static const Manual manual;

	const ManualItemHeading &heading = manual.getHeading(item);

	std::ostringstream ss;
	ss
		<< "[see "
		<< manualFileName
		<< ": "
		<< heading.number
		<< " "
		<< heading.title
		<< "]"
	;

	return ss.str();
}
