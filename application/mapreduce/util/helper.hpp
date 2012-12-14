// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_HELPER
#define _H_MAPREDUCE_UTIL_HELPER 1

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <fhglog/fhglog.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <fvm-pc/pc.hpp>
#include <util/types.hpp>
#include <algorithm>
#include <boost/regex.hpp>
#include <sys/time.h>
#include <ctime>

const int US = 1000000.0L;
const int MS = 1000.0L;
const int KEY_MAX_SIZE = 50;

const char SHRPCH 	= '#';
const char NLCH 	= '\n';
const char SPCH 	= ' ';
const char PAIRSEP 	= '@';

std::string DELIMITERS = " \n";

typedef  std::pair<std::string, std::string> key_val_pair_t;
typedef unsigned long long timestamp_t;

namespace mapreduce
{
  namespace util
  {
	  void insert_element(vector<std::string>& arr_items, std::string element )
	  {
		  size_t size = arr_items.size();
		  arr_items.resize(size+1);
		  int i = size;

		  for(; i > 0 && arr_items[i-1]>element; --i)
		  {
			  arr_items[i] = arr_items[i-1];
		  }

		  arr_items[i] = element;
	  }

  	  std::string make_spec_left_prefix(const long& chunk_id)
  	  {
  		  std::ostringstream oss;
  		  oss<<SHRPCH<<chunk_id<<SHRPCH<<"0";
  		  return oss.str();
  	  }

  	  std::string make_spec_right_prefix(const long& chunk_id)
	  {
  		  std::ostringstream oss;
  		  oss<<SHRPCH<<chunk_id<<SHRPCH<<"1";
  		  return oss.str();
	  }

  	  void my_sort(std::vector<std::string>::iterator iter_beg, std::vector<std::string>::iterator iter_end )
  	  {
  		  std::sort(iter_beg, iter_end);
  	  }

  	  template <typename T>
  	  void print_keys(const std::string& msg, T& arr_keys )
  	  {
  		  std::ostringstream osstr;

  		  int k=0;
  		  for( typename T::iterator it = arr_keys.begin(); it!= arr_keys.end(); it++ )
  		  {
  			  osstr<<*it<<" ";
  			  if(k++%4 == 0)
  				  osstr<<std::endl;
  		  }

  		  MLOG(INFO, msg<<" "<<osstr.str());
  	  }


  	  std::string match_keys(const std::string& keyval_pair, const std::list<std::string>& list_border_keys, std::string& matching_pair, int& cid, int& end)
  	  {
  		  std::string w;
  		  bool bMatching = false;

  		  //MLOG(INFO, "Trying to find the matching pair of the item "<<keyval_pair<<" ...");
  		  std::ostringstream oss;
  		  oss<<SHRPCH<<PAIRSEP;
  		  boost::char_separator<char> sep(oss.str().data());
  		  boost::tokenizer<boost::char_separator<char> > tok_v(keyval_pair, sep);
  		  std::vector<std::string> v(3,"");
  		  v.assign(tok_v.begin(), tok_v.end());

  		  cid = boost::lexical_cast<long>(v[0]);
  		  end = boost::lexical_cast<long>(v[1]);

  		  if( list_border_keys.empty() )
  		  {
  			  MLOG(WARN, "The array of border keys is empty ...");
  			  matching_pair = "";
  			  return "";
  		  }

  		  for(std::list<std::string>::const_iterator it = list_border_keys.begin(); it != list_border_keys.end(); it++ )
  		  {
  			  boost::tokenizer<boost::char_separator<char> > tok_u(*it, sep);
  			  std::vector<std::string> u(3, "");
  			  u.assign(tok_u.begin(), tok_u.end());

  			  int v0 = cid;
  			  int u0 = boost::lexical_cast<long>(u[0]);

  			  int v1 = end;
  			  int u1 = boost::lexical_cast<long>(u[1]);

  			  if( v0 == u0+1 && u1 == v1+1 )
  			  {
  				  w = u[2]+v[2];
  				  bMatching = true;
  				  matching_pair = *it;
  			  }
  			  else
  				  if( u0 == v0+1 && v1 == u1+1 )
  				  {
  					  w = v[2]+u[2];
  					  bMatching = true;
  					  matching_pair = *it;
  				  }
  		  }

  		  if(w.empty())
  			  MLOG(INFO, "No matching pair was found!");

  		  return w;
  	  }


  	  bool is_special_item(const std::string& str_item)
  	  {
  		  if (str_item[0] != SHRPCH)
  			  return false;

  		  // further checks are necessary
  		 std::ostringstream oss;
  		 oss<<SHRPCH<<PAIRSEP;
  		 boost::char_separator<char> sep(oss.str().data());
  		 boost::tokenizer<boost::char_separator<char> > tok(str_item, sep);
  		 std::vector<std::string> u(3, "");
  		 u.assign(tok.begin(), tok.end());

  		 try {
  			  int u0 = boost::lexical_cast<long>(u[0]);
  			  int u1 = boost::lexical_cast<long>(u[1]);
  			  return true;
  		 }
  		 catch(const boost::bad_lexical_cast& ex)
  		 {
  			 return false;
  		 }

  		 // const boost::regex pattern("#\\d+#\\d+:\\w*");
  		 // return boost::regex_match(str_item, pattern);
  	  }

    bool is_delimiter(char x)
    {
      bool b = (DELIMITERS.find(x) != std::string::npos);
      return b;
    }

     bool string_comp( const std::string &left, const std::string &right )
     {
        for( std::string::const_iterator lit = left.begin(), rit = right.begin(); lit != left.end() && rit != right.end(); ++lit, ++rit )
          if( *lit < *rit )
            return true;
          else if( *lit > *rit  )
            return false;

        if( left.size() < right.size() )
          return true;

        return false;
    }

    bool comp (const std::string& l, const std::string& r)
    {
      size_t l_pos = l.find(PAIRSEP);
      size_t r_pos = l.find(PAIRSEP);
      std:: string l_pref = l.substr(0, l_pos);
      std:: string r_pref = r.substr(0, r_pos);

      return string_comp(l_pref, r_pref);
    }

    std::vector<int> get_array(const std::string& str_input)
    {
      boost::char_separator<char> sep(DELIMITERS.c_str());
      boost::tokenizer<boost::char_separator<char> > tok(str_input, sep);
      std::vector<std::string> v;
      v.assign(tok.begin(),tok.end());

      std::vector<int> vint;

      BOOST_FOREACH(std::string& strval, v)
        vint.push_back(boost::lexical_cast<int>(strval));

      return vint;
    }

    template <typename T>
    std::string get_string(typename std::vector<T>& arr)
    {
       std::stringstream sstr;
       for(typename std::vector<T>::iterator it=arr.begin();it!=arr.end();it++)
       {
          sstr<<*it<<" ";
       }

       return sstr.str();
    }

    std::string get_string(std::vector<int>::iterator& it_first,  std::vector<int>::iterator& it_last)
    {
       std::stringstream sstr;
       for(std::vector<int>::iterator it = it_first; it != it_last; it++)
       {
          sstr<<*it<<" ";
       }

       return sstr.str();
    }

    std::vector<std::string> get_list_items(char* local_buff)
    {
      std::string str_buff(local_buff);
      boost::char_separator<char> sep(DELIMITERS.c_str());
      boost::tokenizer<boost::char_separator<char> > tok(str_buff, sep);
      std::vector<std::string> v;
      v.assign(tok.begin(),tok.end());

      return v;
    }

    std::string gen_all_reduce_slots(const int& avail_slots)
    {
      std::stringstream sstr;
      for(int k=0;k<avail_slots;k++)
      {
        sstr<<k;
        if(k<avail_slots-1)
          sstr<<" ";
      }

      return sstr.str();
    }

    template <typename T>
    void dump_iter_red_info(const std::string& name, const T& t)
    {
      std::stringstream sstr;
      sstr<<"   name:"<<name<<std::endl;
      sstr<<"   avail_slots = "<<t.avail_slots<<std::endl;
      sstr<<"   slots_to_reduce = "<<t.slots_to_reduce<<std::endl;
      sstr<<"   part_used = "<<t.part_used<<std::endl;
      sstr<<"   red_used = "<<t.red_used<<std::endl;
      sstr<<"   border_used = "<<t.border_used<<std::endl;

      MLOG(INFO, sstr.str());
    }

    void print_partitions(long handle, long slot_size, const std::string& part_used)
    {
      /*************************************************************************************************************************/
      std::vector<int> arr_used = ::mapreduce::util::get_array(part_used);
      for(int k=0; k<arr_used.size(); k++)
      {
        long vm_part_offset = k*slot_size;

        char* ptr_shmem = static_cast<char *> (fvmGetShmemPtr());
        bzero(ptr_shmem, arr_used[k]+1);

        waitComm ( fvmGetGlobalData
        ( static_cast<fvmAllocHandle_t> (handle)
                          , vm_part_offset
                          , arr_used[k]
                          , 0
                          , 0
                        )
        );

        std::vector<std::string> arr_items = ::mapreduce::util::get_list_items(ptr_shmem);
        MLOG(INFO,"Partition "<<k<<", of size "<<arr_used[k]<<", contains "<<arr_items.size()<<" items: "<<ptr_shmem<<std::endl);
      }
      /*************************************************************************************************************************/
    }

    key_val_pair_t get_key_val(const std::string& str_map)
    {
      size_t split_pos = str_map.find_last_of(PAIRSEP);
      std::string key = str_map.substr(0,split_pos);
      std::string val = str_map.substr(split_pos+1, str_map.size());

      return key_val_pair_t(key, val);
    }

    std::string make_string(const key_val_pair_t& pair)
    {
    	std::ostringstream osstr;
    	osstr<<pair.first<<PAIRSEP<<pair.second;

    	return osstr.str();;
    }

    long ceil(long a, long b)
    {
    	int rest = (a%b>0)?1:0;
    	return (a/b)+rest;
    }

    timestamp_t get_timestamp()
	{
    	struct timeval now;
    	gettimeofday (&now, NULL);
    	return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
	}

  }
}

#endif
