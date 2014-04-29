#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include "IgnoredData.h"
#include "Config.h"
#include "DiffState.h"
#include "Database.h"
#include "Outs.h"
#include "Scripts.h"
#include "Temp.h"

int main();
void main_inner();
Config createConfig(const DatabaseTypes &databaseTypes);
void ensureIgnoreFilesExist();
std::set<std::string> loadIgnoredTables();
std::vector<IgnoredData> loadIgnoredData();
IgnoredData parseIgnoredDataLine(const std::string &line);
void clearDatabase(Database &db);
void importScripts(Database &db, const Scripts &scripts);
void ensureIsVersioned(Database &db);
void ensureNotHigherVersionThan(const uint32_t maxAllowedVersion, Database &db);
void applyMissingScripts(Database &db, const Scripts &scripts);
SortedTables sortTables(Database &dbLocal, Database &dbTemp, std::set<std::string> &ignoredTables);
std::vector<std::string> sortByDependencies(const std::vector<std::string> &tables, Database &db);
Different handleBoth(const std::vector<std::string> &tables, Database &dbLocal, Database &dbTemp, Outs &outs, std::vector<IgnoredData> &ignoredData, Temp &temp);
boost::optional<std::string> getIgnoredDataWhere(const std::string &tableName, const std::vector<IgnoredData> &ignoredDataList);
void handleRepositoryOnly(const std::vector<std::string> &tables, uint32_t &nextScriptTargetVersion, Database &dbLocal, Outs &outs);
void handleLocalOnly(const std::vector<std::string> &tables, uint32_t &nextScriptTargetVersion, Database &dbLocal, Outs &outs, std::vector<IgnoredData> &ignoredData, Temp &temp);
void printDifferences(const SortedTables &sortedTables, const Different &different);
