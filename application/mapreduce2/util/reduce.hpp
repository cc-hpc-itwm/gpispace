// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_REDUCE
#define _H_MAPREDUCE_UTIL_REDUCE 1

#include <sstream>
#include <stdexcept>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <fhglog/fhglog.hpp>
#include <fstream>
#include <list>
#include <stdexcept>

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
        	//MLOG(INFO, "Reduce the item "<<str_val);

        	boost::char_separator<char> sep("[] ");
        	boost::tokenizer<boost::char_separator<char> > tok(str_val, sep);
        	std::vector<std::string> v;
        	v.assign(tok.begin(), tok.end());

        	for(int k=0;k<v.size();k++)
        		total += boost::lexical_cast<int>(v[k]);
        }

       // MLOG(INFO, "After reduction, "<<key<<" -> "<<total<<" ");
        std::string str_total(boost::lexical_cast<std::string>(total));
        list_out_values.push_back(str_total);

        return list_out_values;
      }

      size_t store(std::string& key, std::list<std::string>& list_values, char* reduce_buff, size_t last_pos, const long& n_max_size)
      {
        std::stringstream sstr;
        sstr<<key<<":[";

        for( std::list<std::string>::iterator it=list_values.begin(); it!=list_values.end(); it++ )
        {
          sstr<<*it;

          if( boost::next(it) != list_values.end() )
            sstr<<" ";
          else
            sstr<<"] ";
        }

        size_t item_size = sstr.str().size();
        if(last_pos+item_size>n_max_size)
        {
          throw(std::runtime_error("Not enough place left for performing a reduce operation!"));
        }
        else
        {
          memcpy(reduce_buff+last_pos, sstr.str().data(), item_size);
          size_t new_pos = last_pos + item_size;

          return new_pos;
        }
      }

    void write(std::string& key, std::list<std::string>& list_values, std::ofstream& ofs )
	{
		ofs<<key<<":[";

		for( std::list<std::string>::iterator it=list_values.begin(); it!=list_values.end(); it++ )
		{
			ofs<<*it;

			if( boost::next(it) != list_values.end() )
				ofs<<" ";
			else
				ofs<<"] "<<std::endl;
		}
	}
  }
}

#endif
