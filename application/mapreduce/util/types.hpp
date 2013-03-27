// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_TYPES
#define _H_MAPREDUCE_UTIL_TYPES 1

#include <sstream>
#include <string>
#include <sstream>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

using namespace std;

namespace mapreduce
{
  namespace util
  {
    typedef std::pair<std::string, std::string> key_val_pair_t;

    /*class key_val_pair_t
    {
    public:

    	key_val_pair_t(const std::string& inKey = "", const std::string& inVal = "")
      	  : first(inKey),
      	    second(inVal)
      	  {}

    	void print()
    	{
    		std::cout<<key()<<":"<<val();
    	}

    	template<class Archive>
    	void serialize(Archive & ar, const unsigned int )
    	{
    		ar & first;
    		ar & second;
    	}

    	std::string key() { return first; }
    	std::string val() { return second; }

    	std::string set_key(const std::string& key) { first=key; }
    	std::string set_val(const std::string& val) { second=val; }

    	std::string encode()
    	{
    		std::stringstream osstr;
    		boost::archive::binary_oarchive ar(osstr);
    		ar << *this;
    		return osstr.str();
    	}

    	void decode (const std::string& strenc)
    	{
    		std::stringstream sstr(strenc);
    		boost::archive::binary_iarchive ar(sstr);
    		ar >> *this;
    	}

    	public: // make it later private

    	std::string first;
    	std::string second;
    };*/

    typedef std::list<key_val_pair_t> list_key_val_pairs_t;
  }
}

#endif
