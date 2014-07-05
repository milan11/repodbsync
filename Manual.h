#pragma once

#include <map>
#include <set>
#include <string>

enum class ManualItem {
    SETTINGS_FILE,
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
    typedef std::map<ManualItem, ManualItemHeading> ItemsMap;
    ItemsMap items;

};

std::string seeManual(const ManualItem item);
