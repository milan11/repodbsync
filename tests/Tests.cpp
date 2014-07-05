
#define BOOST_TEST_NO_MAIN // we define our own main method below which disables system errors checks (adding to argv)
#define BOOST_TEST_MAIN // we have to define this to have ::boost::unit_test::unit_test_main available

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

boost::filesystem::path executablePath;
boost::filesystem::path manualFilePath;
bool skipManualTests;

boost::filesystem::path getExecutableDirectory(const std::string &firstArgv) {
    if (! firstArgv.empty()) {
        boost::filesystem::path firstArgvPath = firstArgv;
        if (firstArgvPath.is_absolute()) {
            return firstArgvPath.parent_path();
        }
        if (firstArgvPath.is_relative()) {
            return (boost::filesystem::current_path() / firstArgvPath).parent_path();
        }
    }
    return boost::filesystem::current_path();
}

void loadManualArguments(int argc, char* argv[]) {
    static const std::string manualFileArg = "--manual_file=";
    static const std::string skipManualTestsArg = "--skip_manual_tests";

    skipManualTests = false;

    for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.find(manualFileArg) == 0) {
            manualFilePath = arg.substr(manualFileArg.size());
        }
        if (arg.find(skipManualTestsArg) == 0) {
            skipManualTests = true;
        }
    }

}

int BOOST_TEST_CALL_DECL
main(int argc, char *argv[]) {
    executablePath = getExecutableDirectory((argc > 0) ? argv[0] : "");
    loadManualArguments(argc, argv);

    std::vector<char *> modifiedArgv;
    modifiedArgv.resize(static_cast<unsigned int>(argc) + 1);

    std::copy(argv, argv + argc, &modifiedArgv[0]);

    const char *systemErrorsNoStr = "--catch_system_errors=no";
    const size_t systemErrorsNoStrLength = ::strlen(systemErrorsNoStr) + 1;
    std::vector<char> systemErrorsNo;
    systemErrorsNo.resize(systemErrorsNoStrLength);
    std::copy(systemErrorsNoStr, systemErrorsNoStr + systemErrorsNoStrLength, &systemErrorsNo[0]);

    modifiedArgv[static_cast<unsigned int>(argc)] = &systemErrorsNo[0];

    return ::boost::unit_test::unit_test_main( &init_unit_test, static_cast<int>(modifiedArgv.size()), &modifiedArgv[0] );
}
