#include "IgnoredData.h"

IgnoredData::IgnoredData(std::string tableName, std::string where)
	:
	  tableName(std::move(tableName)),
	  where(std::move(where))
{
}

const std::string &IgnoredData::getTableName() const {
	return tableName;
}

const std::string &IgnoredData::getWhere() const {
	return where;
}
