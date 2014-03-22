#pragma once

#include <string>

class IgnoredData {
public:
	IgnoredData(std::string tableName, std::string where);

	const std::string &getTableName() const;
	const std::string &getWhere() const;

private:
	const std::string tableName;
	const std::string where;

};
