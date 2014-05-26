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
std::set<std::string> loadIgnoredRoutines();
IgnoredData parseIgnoredDataLine(const std::string &line);
void importScripts(Database &db, const Scripts &scripts);
void ensureIsVersioned(Database &db);
void ensureNotHigherVersionThan(const uint32_t maxAllowedVersion, Database &db);
void applyMissingScripts(Database &db, const Scripts &scripts);
SortedTables sortTables(Database &dbLocal, Database &dbTemp, std::set<std::string> &ignoredTables);
std::vector<std::string> sortByDependencies(const std::vector<std::string> &tables, Database &db);
DifferentTables handleTables_both(const std::vector<std::string> &tables, Database &dbLocal, Database &dbTemp, Outs &outs, std::vector<IgnoredData> &ignoredData, Temp &temp);
boost::optional<std::string> getIgnoredDataWhere(const std::string &tableName, const std::vector<IgnoredData> &ignoredDataList);
void handleTables_repositoryOnly(const std::vector<std::string> &tables, uint32_t &nextScriptTargetVersion, Database &dbLocal, Outs &outs);
void handleTables_localOnly(const std::vector<std::string> &tables, uint32_t &nextScriptTargetVersion, Database &dbLocal, Outs &outs, std::vector<IgnoredData> &ignoredData, Temp &temp);
SortedRoutines sortRoutines(Database &dbLocal, Database &dbTemp, std::set<std::string> &ignoredRoutines);
std::vector<std::string> handleRoutines_both(const std::vector<std::string> &routines, Database &dbLocal, Database &dbTemp, Outs &outs, Temp &temp);
void handleRoutines_repositoryOnly(const std::vector<std::string> &routines, uint32_t &nextScriptTargetVersion, Database &dbLocal, Outs &outs);
void handleRoutines_localOnly(const std::vector<std::string> &routines, uint32_t &nextScriptTargetVersion, Database &dbLocal, Outs &outs);
void printDifferences(const SortedTables &sortedTables, const DifferentTables &differentTables, const SortedRoutines &sortedRoutines, const std::vector<std::string> &differentRoutines);
