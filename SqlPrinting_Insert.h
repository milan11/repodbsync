#pragma once

#include <ostream>
#include "SqlStructures_Insert.h"

void printInsert(const sql::insert::Insert &insert, std::ostream &s);
