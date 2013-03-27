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
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/utility.hpp> // for std::pair
#include <boost/serialization/vector.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <util/types.hpp>
#include <algorithm>
#include <boost/regex.hpp>
#include <util/time.hpp>
#include <fstream>
#include <stdlib.h>

const int US = 1000000;
const int MS = 1000;
const int KEY_MAX_SIZE = 50;
#define KEY_LENGTH 2

const char SHRPCH = '#';
const char NLCH = '\n';
const char SPCH = ' ';
const char PAIRSEP = '@';

std::string DELIMITERS = " \n";

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

  	  std::string key(const std::string& item)
  	  {
  		  size_t split_pos = item.find_last_of(PAIRSEP);
  		  std::string key = item.substr(0,split_pos);
  		  return key;
  	  }

  	  std::string val(const std::string& item)
  	  {
  		 size_t split_pos = item.find_last_of(PAIRSEP);
  		 std::string val = item.substr(split_pos+1);
  		 return val;
  	  }

  	  key_val_pair_t str2kvpair(const std::string& str_map)
  	  {
  		  size_t split_pos = str_map.find_last_of(PAIRSEP);
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

  	  std::string kvpair2str(const key_val_pair_t& pair)
  	  {
  		  return pair.first+PAIRSEP+pair.second;
  	  }

  	  bool is_special_key(const std::string& key)
  	  {
  		  if(key[0] != SHRPCH)
  			  return false;

  		  char szsep[2];
  		  szsep[0]=SHRPCH;
  		  szsep[1]='\0';
  		  boost::char_separator<char> sep(szsep);
  		  boost::tokenizer<boost::char_separator<char> > tok(key, sep);
  		  std::vector<std::string> u(2, "");
  		  u.assign(tok.begin(), tok.end());

  		  try {
  			  (void)(boost::lexical_cast<long>(u[0]));
  			  (void)(boost::lexical_cast<long>(u[1]));
  			  return true;
  		  }
  		  catch(const boost::bad_lexical_cast& ex) {
  			  return false;
  		  }
  	  }

  	  bool my_comp_pairs(::mapreduce::util::key_val_pair_t lhs, ::mapreduce::util::key_val_pair_t rhs)
  	  {
  		  return lhs.first.compare(rhs.first)<0;
  	  }

  	  bool my_comp(const std::string& lhs, const std::string& rhs)
  	  {
  		  /*std::string key_l(lhs);
			std::string key_r(rhs);*/

  		  std::string key_l(key(lhs));
  		  std::string key_r(key(rhs));

  		  return key_l.compare(key_r)<0;
  	  }

  	  void my_sort(std::vector<std::string>::iterator iter_beg, std::vector<std::string>::iterator iter_end )
  	  {
  		  std::sort(iter_beg, iter_end, my_comp);
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

  	  std::string match_keys(const std::string& str_item, const std::list<std::string>& list_border_keys, std::string& matching_pair, int& cid, int& end)
  	  {
  		 // to do: use regex here
		 std::string w;
		 bool bMatching = false;

		 // further checks are necessary
		 key_val_pair_t kvp_v = str2kvpair(str_item);
		 char szsep[2];
		 szsep[0]=SHRPCH;szsep[1]='\0';
		 boost::char_separator<char> sep(szsep);
		 boost::tokenizer<boost::char_separator<char> > tok_v(kvp_v.first, sep);
		 std::vector<std::string> v(2,"");
		 v.assign(tok_v.begin(), tok_v.end());

		 cid = atoi(v[0].c_str()); //boost::lexical_cast<long>(v[0]);
		 end = atoi(v[1].c_str()); //boost::lexical_cast<long>(v[1]);

		 if( list_border_keys.empty() )
		 {
			 LOG(WARN, "The array of border keys is empty ...");
			 matching_pair = "";
			 return "";
		 }

		 for(std::list<std::string>::const_iterator it = list_border_keys.begin(); it != list_border_keys.end(); it++ )
		 {
			 key_val_pair_t kvp_u = str2kvpair(*it);
			 boost::tokenizer<boost::char_separator<char> > tok_u(kvp_u.first, sep);
			 std::vector<std::string> u(2, "");
			 u.assign(tok_u.begin(), tok_u.end());

			 int v0 = cid;
			 int u0 = atoi(u[0].c_str()); //boost::lexical_cast<long>(u[0]);

			 int v1 = end;
			 int u1 = atoi(u[1].c_str()); //boost::lexical_cast<long>(u[1]);

			 if( v0 + v1 == u0 + u1 )
			 {
				 w = (u0<v0)?kvp_u.second + kvp_v.second : w = kvp_v.second + kvp_u.second;
				 bMatching = true;
				 matching_pair = *it;
			 }
		 }

		 if(w.empty())
		 {
			 std::ostringstream oss;
			 oss<<"{";
			 BOOST_FOREACH(const std::string& key, list_border_keys)
			 {
				 oss<<key<<std::endl;
			 }
			 oss<<"}";

			 MLOG(INFO, "No matching pair was found! The list of border keys is: "<<oss.str());
		 }

		 return w;
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


	 void get_arr_pairs(char* local_buff, std::vector<key_val_pair_t>& arr_pairs)
	{
    	std::string str_buff(local_buff);
    	boost::char_separator<char> sep(DELIMITERS.c_str());
    	boost::tokenizer<boost::char_separator<char> > tok(str_buff, sep);

        BOOST_FOREACH (const std::string& s, tok)
        {
          arr_pairs.push_back(str2kvpair(s));
        }
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

   std::string get_generic_part_filename(const std::string& cfg_out)
   {
	   std::vector<std::string> name_and_ext;
		boost::split(name_and_ext, cfg_out, boost::is_any_of("."));
		std::ostringstream sstr_part_out_file;
		sstr_part_out_file<<name_and_ext[0]<<"_*";
		sstr_part_out_file<<"."<<name_and_ext[1];

		return sstr_part_out_file.str();
   }

   size_t write_to_buff(std::string& key, std::list<std::string>& list_values, char* reduce_buff, size_t last_pos, const size_t& n_max_size)
   {
	   std::stringstream sstr;

	   key_val_pair_t kvp(key, list2str(list_values));
	   std::string str_pair = kvpair2str(kvp);
	   size_t item_size = str_pair.size();

	   if(last_pos+item_size>n_max_size)
	   {
		   throw(std::runtime_error("Not enough shared memory!"));
	   }
	   else
	   {
		   memcpy(reduce_buff+last_pos, str_pair.data(), item_size);
		   size_t new_pos = last_pos + item_size;
		   reduce_buff[new_pos++]=SPCH;

		   return new_pos;
	   }
   }

	size_t write_to_buff(const std::string& str_pair, char* reduce_buff, size_t last_pos, const size_t& n_max_size, const int sp=SPCH )
	{
	  std::stringstream sstr;
	  size_t item_size = str_pair.size();

	  if(last_pos+item_size>n_max_size)
	  {
		  throw(std::runtime_error("Not enough shared memory!"));
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

	void write_to_stream(std::string& key, std::list<std::string>& list_values, std::ofstream& ofs )
	{
		key_val_pair_t kvp(key, list2str(list_values));
		std::string str_pair = kvpair2str(kvp);
		ofs<<str_pair<<std::endl;
	}

	void write_arr_to_buff( const std::vector<std::string>& arr_items, char* ptr_shmem, size_t& last_pos, const size_t max_size )
	{
		for(std::vector<std::string>::const_iterator it=arr_items.begin(); it != arr_items.end(); it++ )
			last_pos = ::mapreduce::util::write_to_buff( *it, ptr_shmem, last_pos, max_size );
	}

	void serialize_kv_pair_array( const std::vector<key_val_pair_t> & arr_items, char* buffer, size_t buffer_size )
	{
		boost::iostreams::basic_array_sink<char> sr(buffer, buffer_size);
		boost::iostreams::stream< boost::iostreams::basic_array_sink<char> > source(sr);
		boost::archive::binary_oarchive oa(source);
		oa << arr_items;
	}

	void serialize_kv_pair_array( const std::vector<key_val_pair_t> & arr_items, std::string& serial_str )
	{
		// serialize obj into an std::string
		boost::iostreams::back_insert_device<std::string> inserter(serial_str);
		boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(inserter);
		boost::archive::binary_oarchive oa(s);
		oa << arr_items;
	}

	void deserialize_kv_pair_array(const std::string& serial_str, std::vector<key_val_pair_t>& arr_items )
	{
		// serialize obj into an std::string
		boost::iostreams::basic_array_source<char> device(serial_str.data(), serial_str.size());
		boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
		boost::archive::binary_iarchive ia(s);
		ia >> arr_items;
	}

	void get_list_str_items_from_pairs(const std::vector<key_val_pair_t>& arr_kv_part_items,  std::vector<std::string>& arr_part_items )
	{
		BOOST_FOREACH(key_val_pair_t kv_pair, arr_kv_part_items)
		{
			arr_part_items.push_back(kvpair2str(kv_pair));
		}
	}
  }
}

#endif
