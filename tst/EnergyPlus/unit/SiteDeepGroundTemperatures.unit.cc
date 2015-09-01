// EnergyPlus::GroundTemperatureModels Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "EnergyPlus/DataIPShortCuts.hh"
#include "Fixtures/GroundTempsFixture.hh"

using namespace EnergyPlus;

using namespace EnergyPlus::GroundTemperatureManager;
 
TEST_F( GroundTempsFixture, SiteDeepGroundTempTest )
{
	std::string const idf_objects = delimited_string({
		"Version,8.4;",
		"Site:GroundTemperature:Deep,",
		"	21.00,	!- January",
		"	22.00,	!- February",
		"	23.00,	!- March",
		"	24.00,	!- April",
		"	25.00,	!- May",
		"	26.00,	!- June",
		"	27.00,	!- July",
		"	28.00,	!- August",
		"	29.00,	!- Septeber",
		"	30.00,	!- October",
		"	31.00,	!- November",
		"	32.00;	!- December",
	});

	ASSERT_FALSE( process_idf( idf_objects ) );
	
	std::string const CurrentModuleObject = CurrentModuleObjects( objectType_SiteDeepGroundTemp );

	auto thisModel = GetGroundTempModelAndInit( CurrentModuleObject, "TEST" );

	EXPECT_NEAR( 21.0, thisModel->getGroundTempAtTimeInMonths( 0.0, 1 ), 0.1 );		// January
	EXPECT_NEAR( 32.0, thisModel->getGroundTempAtTimeInMonths( 0.0, 12 ), 0.1 );	// December
	EXPECT_NEAR( 22.0, thisModel->getGroundTempAtTimeInMonths( 0.0, 14 ), 0.1 );	// Feb of next year

	EXPECT_NEAR( 23.0, thisModel->getGroundTempAtTimeInSeconds( 0.0, 6393600 ), 0.1 );	// March 15
	EXPECT_NEAR( 29.0, thisModel->getGroundTempAtTimeInSeconds( 0.0, 22291200 ), 0.1 );	// Sept 15
	EXPECT_NEAR( 22.0, thisModel->getGroundTempAtTimeInSeconds( 0.0, 35510400 ), 0.1 ); // Feb 15 of next year
}
