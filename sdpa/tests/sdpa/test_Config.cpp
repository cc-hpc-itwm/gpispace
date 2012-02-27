#define BOOST_TEST_MODULE TestConfig
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>

#include <sdpa/util/util.hpp>
#include <sdpa/util/Config.hpp>
#include <sdpa/logging.hpp>

using namespace sdpa;
using boost::property_tree::ptree;

struct MyFixture
{
	MyFixture() :SDPA_INIT_LOGGER("sdpa.tests.testConfig"){}
	~MyFixture(){}
	 SDPA_DECLARE_LOGGER();
};

BOOST_FIXTURE_TEST_SUITE( test_PropTree, MyFixture )

BOOST_AUTO_TEST_CASE( testWriteIni )
{
	std::cout<<"-----------------testWriteIni---------------"<<std::endl;
	ptree root;
    ptree wave_packet;

    wave_packet.put( "width", "1" );
    wave_packet.put( "position", "2.0" );

    ptree calculation_parameters;
    calculation_parameters.put( "levels", "15" );
    calculation_parameters.put<double>( "stuff", 23.888 );

    root.push_front( ptree::value_type( "calculation parameters", calculation_parameters ) );
    root.push_front( ptree::value_type( "wave packet", wave_packet ));

    write_ini( "config.txt", root );
}

BOOST_AUTO_TEST_CASE( testReadIni )
{
	std::cout<<"-----------------testReadIni----------------"<<std::endl;
	std::string configFile("config.txt");
	ptree propTree;
	read_ini(configFile, propTree);

	write_ini( std::cout, propTree );
}


BOOST_AUTO_TEST_CASE( testGetPropVal )
{
	std::cout<<"-----------------testGetPropVal-------------"<<std::endl;
	std::string configFile("config.txt");
	ptree propTree;
	read_ini(configFile, propTree);

	double x = propTree.get<double>("calculation parameters.stuff");
	std::cout<<"Stuff = "<<x;
}

BOOST_AUTO_TEST_SUITE_END()

