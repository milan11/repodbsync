#pragma once

#include <boost/utility/value_init.hpp>
#include <cstdint>
#include <vector>
#include "SqlStructures.h"

namespace sql {
namespace insert {

struct Item {
	Column column;
	Literal value;
	explicit Item(Column column);
};

typedef std::vector<Item> Items;

struct Insert {
	std::string table;
	Items items;

	void setTable(std::string table);
	void addColumn(Column column);
	void addValue(Literal value);

private:
	boost::value_initialized<uint32_t> nextValueIndex;
};

}
}
