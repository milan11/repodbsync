#include "SqlPrinting_Insert.h"

#include "SqlPrinting.h"

void printInsert(const sql::insert::Insert &insert, std::ostream &s) {
	s
		<< "INSERT INTO "
		<< insert.table
		<< " ("
	;

	{
		bool first = true;
		for (const sql::insert::Item &item : insert.items) {
			if (first) {
				first = false;
			} else {
				s << ", ";
			}
			s << item.column;
		}
	}

	s << ") VALUES (";

	{
		bool first = true;
		for (const sql::insert::Item &item : insert.items) {
			if (first) {
				first = false;
			} else {
				s << ", ";
			}
			::printValue(item.value, s);
		}
	}

	s << ")";
}
