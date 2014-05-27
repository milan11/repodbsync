
#define BOOST_TEST_NO_MAIN // we define our own main method below which disables system errors checks (adding to argv)
#define BOOST_TEST_MAIN // we have to define this to have ::boost::unit_test::unit_test_main available

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
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
