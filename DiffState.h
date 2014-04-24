#pragma once

#include <set>
#include <string>
#include <vector>

struct SortedTables {
	std::vector<std::string> repositoryOnly;
	std::vector<std::string> localOnly;
	std::vector<std::string> both;
};

struct Different {
	std::set<std::string> differentTable;
	std::set<std::string> differentData;
};
