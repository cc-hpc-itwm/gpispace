// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_REDUCE
#define _H_MAPREDUCE_UTIL_REDUCE 1

#include <sstream>
#include <stdexcept>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <list>

using namespace std;

namespace mapreduce
{
  namespace util
  {
      std::list<std::string> reduce(const std::string& key, const std::list<string>& list_in_values)
      {
        std::list<std::string> list_out_values;
        long total = 0;

        BOOST_FOREACH(const std::string& str_val, list_in_values)
        {
          total += boost::lexical_cast<int>(str_val);
        }

        std::string str_total(boost::lexical_cast<std::string>(total));
        list_out_values.push_back(str_total);

        return list_out_values;
      }

      size_t store(std::string& key, std::list<std::string>& list_values, char* reduce_buff, size_t last_pos)
      {
        std::stringstream sstr;
        sstr<<key<<":[";

        for( std::list<std::string>::iterator it=list_values.begin(); it!=list_values.end(); it++ )
        {
          sstr<<*it;

          if( boost::next(it) != list_values.end() )
            sstr<<" ";
          else
            sstr<<"]";
        }

        sprintf( reduce_buff+last_pos, "%s ", sstr.str().c_str() );

        size_t new_pos =  last_pos + sstr.str().size()+1;

        return new_pos;
      }
  }
}

#endif
