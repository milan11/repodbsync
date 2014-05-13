#include "SqlStructures_Insert.h"

#include "exceptions.h"

namespace sql {
namespace insert {

Item::Item(Column column)
	: column(std::move(column))
{
}

void Insert::setTable(std::string table) {
	this->table = std::move(table);
}

void Insert::addColumn(Column column) {
	items.emplace_back(std::move(column));
}

void Insert::addValue(Literal value) {
	if (nextValueIndex >= items.size()) {
		THROW("More values than columns in insert.");
	}

	items[nextValueIndex].value = std::move(value);

	++nextValueIndex;
}

}
}
