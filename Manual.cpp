#include "Manual.h"

#include <algorithm>
#include <sstream>

namespace {
	const std::string manualFileName = "README.md";
}

Manual::Manual()
:
	items{
		{ManualItem::SETTINGS_FILE, {"1.0", "Settings File"}},
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
