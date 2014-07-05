#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem/path.hpp>
#include "../LinesReader.h"
#include "../Manual.h"

extern boost::filesystem::path executablePath;
extern boost::filesystem::path manualFilePath;
extern bool skipManualTests;

namespace {
    bool fileContains(const boost::filesystem::path &file, const std::string &str) {
        LinesReader reader(file);

        while (boost::optional<std::string> line = reader.readLine()) {
            std::string line_trimmed = boost::algorithm::trim_copy_if(*line, boost::is_any_of(" \t#"));
            if (line_trimmed == str) {
                return true;
            }
        }

        return false;
    }
}

BOOST_AUTO_TEST_CASE(manual_references) {
    if (skipManualTests) return;

    boost::filesystem::path manualFile;
    if (! manualFilePath.empty()) {
        manualFile = manualFilePath;
    } else {
        manualFile = executablePath / "README.md";
    }

    BOOST_CHECK_MESSAGE(boost::filesystem::exists(manualFile), boost::format("Manual file not found: %1%. Use --manual_file=<path> to specify other path or --skip_manual_tests to skip all manual file tests.") % manualFile.string());

    const Manual manual;

    const std::set<ManualItem> items = manual.getAll();

    for (const ManualItem item : items) {
        const ManualItemHeading &heading = manual.getHeading(item);
        std::string requiredString = heading.number + " " + heading.title;

        BOOST_REQUIRE_MESSAGE(fileContains(manualFile, requiredString), "Manual does not contain: " + requiredString);
    }
}
