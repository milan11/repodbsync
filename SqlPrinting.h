#pragma once

#include <ostream>
#include "SqlStructures.h"

void printValue(const sql::Value &c, std::ostream &s);
void printLiteral(const sql::Literal &l, std::ostream &s);
