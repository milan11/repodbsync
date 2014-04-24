#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include "IgnoredData.h"
#include "Config.h"
#include "DiffState.h"
#include "DbOperations.h"
#include "Outs.h"
#include "Scripts.h"
#include "Temp.h"

int main();
void main_inner();
Config createConfig();
void ensureIgnoreFilesExist();
std::set<std::string> loadIgnoredTables();
std::vector<IgnoredData> loadIgnoredData();
IgnoredData parseIgnoredDataLine(const std::string &line);
void clearDatabase(DbOperations &db);
void importScripts(DbOperations &db, const Scripts &scripts);
void ensureIsVersioned(DbOperations &db);
void ensureNotHigherVersionThan(const uint32_t maxAllowedVersion, DbOperations &db);
void applyMissingScripts(DbOperations &db, const Scripts &scripts);
SortedTables sortTables(DbOperations &dbLocal, DbOperations &dbTemp, std::set<std::string> &ignoredTables);
std::vector<std::string> sortByDependencies(const std::vector<std::string> &tables, DbOperations &db);
Different handleBoth(const std::vector<std::string> &tables, DbOperations &dbLocal, DbOperations &dbTemp, Outs &outs, std::vector<IgnoredData> &ignoredData, Temp &temp);
boost::optional<std::string> getIgnoredDataWhere(const std::string &tableName, const std::vector<IgnoredData> &ignoredDataList);
void handleRepositoryOnly(const std::vector<std::string> &tables, uint32_t &nextScriptTargetVersion, DbOperations &dbLocal, Outs &outs);
void handleLocalOnly(const std::vector<std::string> &tables, uint32_t &nextScriptTargetVersion, DbOperations &dbLocal, Outs &outs, std::vector<IgnoredData> &ignoredData, Temp &temp);
void printDifferences(const SortedTables &sortedTables, const Different &different);
