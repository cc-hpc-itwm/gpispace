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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fvm-pc/pc.hpp>
#include <util/types.hpp>
#include <algorithm>
#include <boost/regex.hpp>
#include <util/time.hpp>

const int US = 1000000.0L;
const int MS = 1000.0L;
const int KEY_MAX_SIZE = 50;

const char SHRPCH = '#';
const char NLCH = '\n';
const char SPCH = ' ';
const char PAIRSEP = '@';

std::string DELIMITERS = " \n";

typedef  std::pair<std::string, std::string> key_val_pair_t;

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
			  arr_items[i] = arr_items[i-1];

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
  		  // to do: use regex here
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
  		  // to do: use regex here
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
          sstr<<*it<<SPCH;
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

      //std::copy(tok.begin(), tok.end(), std::back_inserter<std::vector<std::string> >(v));

      return v;
    }

    std::vector<std::string> get_list_items_strtok(char* local_buff)
    {
    	// attention, the input string is modified!!!!
    	std::vector<std::string> v;
    	char* pch_curr = strtok(local_buff, DELIMITERS.c_str());
    	while(pch_curr)
    	{
    		v.push_back(pch_curr);
    		pch_curr = strtok(NULL, DELIMITERS.c_str());
    	}

    	return v;
    }

    void print_partitions(long handle, long slot_size, const std::string& part_used)
    {
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
    }

    key_val_pair_t str2kvpair(const std::string& str_map)
    {
      size_t split_pos = str_map.find_last_of(PAIRSEP);
      if( split_pos == std::string::npos )
      {
    	  MLOG(FATAL, "No value specified for the key "<<str_map);
    	  throw std::runtime_error(std::string("Invalid key-value pair:") + str_map);
      }

      std::string key = str_map.substr(0,split_pos);
      if(key.empty())
      {
    	  MLOG(FATAL, "Empty key!!!!!");
    	  throw std::runtime_error(std::string("Invalid key-value pair: ") + str_map);
      }

      std::string str_val = str_map.substr(split_pos+1, str_map.size());

      while( boost::algorithm::starts_with(str_val, "[") && boost::algorithm::ends_with(str_val, "]") )
      {
    	  std::string val = str_val.substr(1, str_val.size()-2);
    	  str_val = val;
      }

      return key_val_pair_t(key, str_val);
    }


    std::string list2str(std::list<std::string>& list_values )
	{
		std::ostringstream oss;
		oss<<"[";

		for( std::list<std::string>::iterator it=list_values.begin(); it!=list_values.end(); it++ )
		{
		  oss<<*it;

		  if( boost::next(it) != list_values.end() )
			  oss<<SPCH;
		  else
			  oss<<"]";
		}

		return oss.str();
	}

    std::string kvpair2str(const key_val_pair_t& pair)
    {
    	std::ostringstream osstr;
    	osstr<<pair.first<<PAIRSEP<<pair.second;

    	return osstr.str();
    }

    long ceil(long a, long b)
    {
    	int rest = (a%b>0)?1:0;
    	return (a/b)+rest;
    }

    std::string get_part_filename(const std::string& cfg_out, const int part_id)
    {
    	std::vector<std::string> name_and_ext;
  		boost::split(name_and_ext, cfg_out, boost::is_any_of("."));
  	    std::ostringstream sstr_part_out_file;
  	    sstr_part_out_file<<name_and_ext[0]<<"_";
  	    sstr_part_out_file<<part_id<<"."<<name_and_ext[1];

  	    return sstr_part_out_file.str();
    }

    size_t write_to_buff(std::string& key, std::list<std::string>& list_values, char* reduce_buff, size_t last_pos, const long& n_max_size)
	{
	  std::stringstream sstr;

	  key_val_pair_t kvp(key, list2str(list_values));
	  std::string str_pair = kvpair2str(kvp);
	  size_t item_size = str_pair.size();

	  if(last_pos+item_size>n_max_size)
	  {
		  throw(std::runtime_error("Not enough place left for performing a reduce operation!"));
	  }
	  else
	  {
		  memcpy(reduce_buff+last_pos, str_pair.data(), item_size);
		  size_t new_pos = last_pos + item_size;
		  reduce_buff[new_pos++]=SPCH;

		  return new_pos;
	  }
	}

	size_t write_to_buff(const std::string& str_pair, char* reduce_buff, size_t const last_pos, const long& n_max_size, const int sp=SPCH )
	{
	  std::stringstream sstr;
	  size_t item_size = str_pair.size();

	  if(last_pos+item_size>n_max_size)
	  {
		  throw(std::runtime_error("Not enough place left for performing a reduce operation!"));
	  }
	  else
	  {
		  memcpy(reduce_buff+last_pos, str_pair.data(), item_size);
		  size_t new_pos = last_pos + item_size;
		  if(sp)
			  reduce_buff[new_pos++]=sp;

		  return new_pos;
	  }
	}


  }
}

#endif
