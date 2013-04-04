// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_REDUCE
#define _H_MAPREDUCE_UTIL_REDUCE 1

#include <sstream>
#include <stdexcept>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <fhglog/fhglog.hpp>
#include <fstream>
#include <list>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <util/time.hpp>
#include <util/helper.hpp>
#include <fhglog/fhglog.hpp>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

namespace mapreduce
{
  namespace util
  {
      std::list<std::string> reduce(const std::string& key, const std::list<string>& list_in_values)
      {
    	  std::list<std::string> list_out_values;
    	  long total = 0;

    	  if( list_in_values.empty() )
    	  {
    		  MLOG(WARN, "could not reduce: key := \"" + key + "\", empty list of values!");
    		  return list_out_values;
    	  }

    	  BOOST_FOREACH(const std::string& str_item, list_in_values)
    	  {
    		  std::string str_val(str_item);
    		  while( boost::algorithm::starts_with(str_val, "[") && boost::algorithm::ends_with(str_val, "]") )
			  {
				  std::string val = str_val.substr(1, str_val.size()-2);
				  str_val = val;
			  }

    		  boost::char_separator<char> sep(",");

    		  boost::tokenizer<boost::char_separator<char> > tok(str_val, sep);
    		  for(boost::tokenizer<boost::char_separator<char> >::iterator it = tok.begin(); it != tok.end(); ++it)
    		  {
				  if(boost::algorithm::starts_with(*it, "[") && boost::algorithm::ends_with(*it, "]") )
				  {
					  std::list<string> list_in_interm_values;
					  list_in_interm_values.push_back(*it);
					  std::list<std::string> list_out_interm_val = reduce(key, list_in_interm_values);
					  total += atol(list_out_interm_val.begin()->c_str()); //boost::lexical_cast<int>(*list_out_interm_val.begin());
				  }
				  else
				  {
					  total += std::atol(it->data()); //boost::lexical_cast<int>(*it);
				  }
    		  }
    	  }

    	  list_out_values.push_back(boost::lexical_cast<std::string>(total));

    	  return list_out_values;
      }

      void reduce_arr_buff(const int part_id, const size_t red_slot_size, const std::vector<std::string>& arr_items, char* ptr_shmem, size_t& last_pos )
      {
    	  std::list<std::string> list_in_values;
    	  key_val_pair_t kv_pair, kv_pair_next;

    	  try {
    		  kv_pair = str2kvpair(arr_items[0]);
    	  }
		  catch(const std::exception& exc ){
			  throw std::runtime_error("Invalid key-value pair ("+ arr_items[0] + ") :" + +exc.what());
		  }

    	  std::string last_key = kv_pair.first;

    	  last_pos = 0;
    	  for(std::vector<std::string>::const_iterator it=arr_items.begin(); it != arr_items.end(); it++ )
    	  {
    		  try {
    			  kv_pair_next = ::mapreduce::util::str2kvpair(*it);
    		  }
    		  catch(const std::exception& exc ){
    			  throw std::runtime_error("Invalid key-value pair ("+ *it + ") :" +exc.what());
    		  }

    		  if( kv_pair_next.first != last_key )
    		  {
    			  std::list<std::string> list_out_values = ::mapreduce::util::reduce(last_key, list_in_values);

    			  try {
    				  last_pos = ::mapreduce::util::write_to_buff( last_key, list_out_values, ptr_shmem, last_pos, red_slot_size );
    			  }
    			  catch(const std::exception& exc) {
    				  throw std::runtime_error("Reduce slot "+boost::lexical_cast<std::string>(part_id)+":"+exc.what());
    			  }

					last_key = kv_pair_next.first;
					list_in_values.clear();
					list_in_values.push_back(kv_pair_next.second);
				}
				else
				  list_in_values.push_back(kv_pair_next.second);
    	  }

		  if(!list_in_values.empty())
		  {
			  std::list<std::string> list_out_values = ::mapreduce::util::reduce( last_key, list_in_values );

			  try {
				last_pos = ::mapreduce::util::write_to_buff( last_key, list_out_values, ptr_shmem, last_pos, red_slot_size);
			  }
			  catch(const std::exception& exc) {
				  throw std::runtime_error("Reduce slot "+boost::lexical_cast<std::string>(part_id)+":"+exc.what());
			  }
		  }
      }

      void reduce_arr_file(const std::vector<std::string>& arr_items,  const std::string& str_out_file_2 )
      {
    	  std::list<std::string> list_in_values;
    	  key_val_pair_t kv_pair = str2kvpair(arr_items[0]);
    	  std::string last_key = kv_pair.first;

    	  std::ofstream ofs( str_out_file_2.data() );

    	  for(std::vector<std::string>::const_iterator it=arr_items.begin(); it != arr_items.end(); it++ )
    	  {
    		  ::mapreduce::util::key_val_pair_t kv_pair_next = ::mapreduce::util::str2kvpair(*it);
    		  std::string key = kv_pair_next.first;

    		  if( key != last_key )
    		  {
    			  std::list<std::string> list_out_values = ::mapreduce::util::reduce(last_key, list_in_values);

    			  try {
    				  ::mapreduce::util::write_to_stream(last_key, list_out_values, ofs );
    			  }
    			  catch(const std::exception& exc) {
    				  ofs.close();
    				  throw std::runtime_error(exc.what());
    			  }

    			  last_key = key;
    			  list_in_values.clear();
    			  list_in_values.push_back(kv_pair_next.second);
    		  }
    		  else
    			  list_in_values.push_back(kv_pair_next.second);
    	  }

    	  if(!list_in_values.empty())
    	  {
    		  std::list<std::string> list_out_values = ::mapreduce::util::reduce(last_key, list_in_values);

    		  try {
    			  ::mapreduce::util::write_to_stream(last_key, list_out_values, ofs );
    		  }
    		  catch(const std::exception& exc) {
    			  ofs.close();
    			  throw std::runtime_error(exc.what());
    		  }
    	  }

    	  ofs.close();
      }

      void combine(const std::vector<std::string>& arr_items, std::vector<std::string>& arr_reduced_items )
	  {
		  std::list<std::string> list_in_values;
		  key_val_pair_t kv_pair, kv_pair_next;

		  try {
			  kv_pair = str2kvpair(arr_items[0]);
		  }
		  catch(const std::exception& exc ){
			  throw std::runtime_error("Invalid key-value pair ("+ arr_items[0] + ") :" + +exc.what());
		  }

		  std::string last_key = kv_pair.first;

		  for(std::vector<std::string>::const_iterator it=arr_items.begin(); it != arr_items.end(); it++ )
		  {
			  try {
				  kv_pair_next = ::mapreduce::util::str2kvpair(*it);
			  }
			  catch(const std::exception& exc ){
				  throw std::runtime_error("Invalid key-value pair ("+ (*it) + ") :" +exc.what());
			  }

			  if( kv_pair_next.first != last_key && !list_in_values.empty() )
			  {
				  std::list<std::string> list_out_values = ::mapreduce::util::reduce(last_key, list_in_values);

				  try {
					  key_val_pair_t kvp(last_key, list2str(list_out_values));
					  std::string str_pair = kvpair2str(kvp);
					  if(!str_pair.empty())
						  arr_reduced_items.push_back(str_pair);
				  }
				  catch(const std::exception& exc) {
					  throw std::runtime_error(std::string("Combine exception: ")+exc.what());
				  }

					last_key = kv_pair_next.first;
					list_in_values.clear();
					list_in_values.push_back(kv_pair_next.second);
				}
				else
					list_in_values.push_back(kv_pair_next.second);
		  }

		  if(!list_in_values.empty())
		  {
			  std::list<std::string> list_out_values = ::mapreduce::util::reduce( last_key, list_in_values );

			  try {
				  key_val_pair_t kvp(last_key, list2str(list_out_values));
				  std::string str_pair = kvpair2str(kvp);
				  if(!str_pair.empty())
					  arr_reduced_items.push_back(str_pair);
				  list_in_values.clear();
			  }
			  catch(const std::exception& exc) {
				  throw std::runtime_error(std::string("Combine exception: ")+exc.what());
			  }
		  }
	  }

      bool file_exists(const std::string& filename)
	  {
    	  return boost::filesystem::exists(filename);
	  }

      std::streampos file_size( const char* file_with_path )
      {
          std::streampos f_size = 0;
          std::ifstream file( file_with_path, std::ios::binary );

          f_size = file.tellg();
          file.seekg( 0, std::ios::end );
          f_size = file.tellg() - f_size;
          file.close();

          return f_size;
      }

    // assume that the both input arrays are sorted reduced !!!!
	size_t merge_and_reduce_arr_arr_2_buff( 	const int part_id,
										const size_t red_slot_size,
										const std::vector<std::string>& arr_items_l,
										const std::vector<std::string>& arr_items_r,
										char* reduce_buff )
	{
		std::list<std::string> list_in_values;
		size_t last_pos = 0;

		MLOG(INFO, "Merge and reduce ...");
		std::vector<std::string>::const_iterator it_l = arr_items_l.begin();
		std::vector<std::string>::const_iterator it_r = arr_items_r.begin();

		// the array and the file are already reduced
		std::string str_pair, curr_item;
		std::string curr_key, last_key, curr_val;
		key_val_pair_t kvp_l, kvp_r;

		while( it_l != arr_items_l.end() || it_r != arr_items_r.end() )
		{
			if( it_l != arr_items_l.end() && it_r != arr_items_r.end() ) //curr_item = my_comp(*it_l, *it_r)?*it_l++:*it_r++;
			{
				kvp_l = str2kvpair(*it_l);
				kvp_r = str2kvpair(*it_r);

				if(kvp_l.first.compare(kvp_r.first)<0)
				{
					curr_item = *it_l++;
					curr_key = kvp_l.first;
					curr_val = kvp_l.second;
				}
				else
				{
					curr_item = *it_r++;
					curr_key = kvp_r.first;
					curr_val = kvp_r.second;
				}
			}
			else//curr_item = (it_r!=arr_items_r.end())?*it_r++:*it_l++;
			{
				if(it_r!=arr_items_r.end())
				{
					kvp_r = str2kvpair(*it_r);
					curr_item = *it_r++;
					curr_key = kvp_r.first;
					curr_val = kvp_r.second;
				}
				else
				{
					kvp_l = str2kvpair(*it_l);
					curr_item = *it_l++;
					curr_key = kvp_r.first;
					curr_val = kvp_r.second;
				}
			}

			key_val_pair_t kv_pair = str2kvpair(curr_item);
			curr_key = kv_pair.first;
			curr_val = kv_pair.second;

			if( curr_key != last_key )
			{
				if( !list_in_values.empty() )
				{
					std::list<std::string> list_out_values =::mapreduce::util::reduce(last_key, list_in_values);
					last_pos = ::mapreduce::util::write_to_buff( last_key, list_out_values, reduce_buff, last_pos, red_slot_size );
					list_in_values.clear();
				}

				last_key = curr_key;
			}

			list_in_values.push_back(curr_val);
		}

		if( !list_in_values.empty() )
		{
			std::list<std::string> list_out_values =::mapreduce::util::reduce(last_key, list_in_values);
			last_pos = ::mapreduce::util::write_to_buff( last_key, list_out_values, reduce_buff, last_pos, red_slot_size );
			list_in_values.clear();
		}

		return last_pos;
	}

	 // assume that the both input arrays are sorted and reduced !!!!
	size_t merge_and_reduce_arr_buff_2_buff(    const int part_id,
													const size_t red_slot_size,
													const std::vector<std::string>& arr_items,
													char* in_buff,
													char* out_buff )
	{
		std::list<std::string> list_in_values;
		size_t last_pos = 0;

		MLOG(INFO, "Merge and reduce ...");
		std::vector<std::string>::const_iterator it = arr_items.begin();
		char* pch = in_buff?strtok(in_buff, DELIMITERS.c_str()):NULL;

		// the array and the file are already reduced
		std::string str_pair, curr_item;
		std::string curr_key, last_key, curr_val;
		key_val_pair_t kvp_l, kvp_r;

		while( it != arr_items.end() || pch )
		{
			if( it != arr_items.end() && pch )
			{
				kvp_l = str2kvpair(*it);
				kvp_r = str2kvpair(pch);

				if( kvp_l.first.compare(kvp_r.first)<0 )
				{
					curr_item = *it++;
					curr_key = kvp_l.first;
					curr_val = kvp_l.second;
				}
				else
				{
					curr_item = pch;
					curr_key = kvp_r.first;
					curr_val = kvp_r.second;
					pch = strtok(NULL, DELIMITERS.c_str());
				}
			}
			else
				if( pch )
				{
					key_val_pair_t kvp_r = str2kvpair(pch);
					curr_item = pch;
					curr_key = kvp_r.first;
					curr_val = kvp_r.second;
					pch = strtok(NULL, DELIMITERS.c_str());
				}
				else
				{
					key_val_pair_t kvp_l = str2kvpair(*it);
					curr_item = *it++;
					curr_key = kvp_l.first;
					curr_val = kvp_l.second;
				}

			if( curr_key != last_key )
			{
				if( !list_in_values.empty() )
				{
					std::list<std::string> list_out_values =::mapreduce::util::reduce(last_key, list_in_values);
					last_pos = ::mapreduce::util::write_to_buff( last_key, list_out_values, out_buff, last_pos, red_slot_size );
					list_in_values.clear();
				}

				last_key = curr_key;
			}

			list_in_values.push_back(curr_val);
		}

		if( !list_in_values.empty() )
		{
			std::list<std::string> list_out_values =::mapreduce::util::reduce(last_key, list_in_values);
			last_pos = ::mapreduce::util::write_to_buff( last_key, list_out_values, out_buff, last_pos, red_slot_size );
			list_in_values.clear();
		}

		return last_pos;
	}

	// assume that the both input arrays are sorted reduced !!!!
	void merge_and_reduce_arr_file(const std::vector<std::string>& arr_items, const std::string& str_out_file)
	{
		std::list<std::string> list_in_values;

		if( !file_exists(str_out_file) )
		{
			 MLOG(INFO, "Create new file "<<str_out_file);
			 reduce_arr_file(arr_items, str_out_file);
			 return;
		}
		MLOG(INFO, "The file "<<str_out_file<<" already exists on disk!!!");

		//else
		if( file_size(str_out_file.data()) == (std::streampos)0 )
		{
			MLOG(INFO, "The reduce file "<<str_out_file<<" exists but has the length 0!");
			reduce_arr_file(arr_items, str_out_file);
			return;
		}

		std::ifstream ifs;
		ifs.open( str_out_file.data() );
		std::string str_tmp_file = str_out_file + std::string(".new");
		std::ofstream ofs( str_tmp_file.data() );

		MLOG(INFO, "Merge and reduce ...");
		std::vector<std::string>::const_iterator it = arr_items.begin();

		char str_curr_line[256];
		ifs.getline(str_curr_line, 256);

		// the array and the file are already reduced
		std::string str_pair, curr_item;
		std::string curr_key, last_key, curr_val;

		while( it != arr_items.end() || !ifs.eof() )
		{
			if( it != arr_items.end() && !ifs.eof() )
			{
				if(	my_comp(*it, str_curr_line) )
					curr_item = *it++;
				else
				{
					curr_item = str_curr_line;
					ifs.getline(str_curr_line, 256);
				}
			}
			else
				if( !ifs.eof() )
				{
					curr_item = str_curr_line;
					ifs.getline(str_curr_line, 256);
				}
				else
					curr_item = *it++;

			key_val_pair_t kv_pair = str2kvpair(curr_item);
			curr_key = kv_pair.first;
			curr_val = kv_pair.second;

			if( curr_key != last_key )
			{
				if( !list_in_values.empty() )
				{
					std::list<std::string> list_out_values =::mapreduce::util::reduce(last_key, list_in_values);
					::mapreduce::util::write_to_stream(last_key, list_out_values, ofs );
					list_in_values.clear();
				}

				last_key = curr_key;
			}

			list_in_values.push_back(curr_val);
		}

		if( !list_in_values.empty() )
		{
			std::list<std::string> list_out_values =::mapreduce::util::reduce(last_key, list_in_values);
			::mapreduce::util::write_to_stream(last_key, list_out_values, ofs );
			list_in_values.clear();
		}

		ifs.close();
		ofs.close();

		std::remove(str_out_file.data());
		std::rename(str_tmp_file.data(), str_out_file.data());
	}

	// assume that the both input arrays are sorted reduced !!!!
	void merge_and_reduce_files(const std::string& str_out_file_1, const std::string& str_out_file_2, const std::string& str_out_file)
	{
		std::list<std::string> list_in_values;
		std::string str_sp;
		str_sp[0]=' ';

		std::ifstream ifs_1;
		std::ifstream ifs_2;

		MLOG(INFO, "Merge and reduce ...");
		char str_curr_line_1[256];
		char str_curr_line_2[256];

		if( !file_exists(str_out_file_1) && !file_exists(str_out_file_2) )
		{
			return;
		}

		if( file_exists(str_out_file_1) )
		{
			ifs_1.open( str_out_file_1.data() );
			ifs_1.getline(str_curr_line_1, 256, NLCH);
		}

		if( file_exists(str_out_file_2) )
		{
			ifs_2.open( str_out_file_2.data() );
			ifs_2.getline(str_curr_line_2, 256, NLCH);
		}


		std::ofstream ofs( str_out_file.data() );

		// the array and the file are already reduced
		std::string str_pair, curr_item;
		std::string curr_key, last_key, curr_val;

		while( !ifs_1.eof() || !ifs_2.eof() )
		{
			if( !ifs_1.eof() && !ifs_2.eof() )
			{
				if(	my_comp(str_curr_line_1, str_curr_line_2) )
				{
					curr_item = str_curr_line_1;
					ifs_1.getline(str_curr_line_1, 256, NLCH);
				}
				else
				{
					curr_item = str_curr_line_2;
					ifs_2.getline(str_curr_line_2, 256, NLCH);
				}
			}
			else
				if( !ifs_2.eof() )
				{
					curr_item = str_curr_line_2;
					ifs_2.getline(str_curr_line_2, 256, NLCH);
				}
				else
				{
					curr_item = str_curr_line_1;
					ifs_1.getline(str_curr_line_1, 256, NLCH);
				}

			if(!(curr_item.empty() || curr_item == str_sp) )
			{
				key_val_pair_t kv_pair = str2kvpair(curr_item);
				curr_key = kv_pair.first;
				curr_val = kv_pair.second;

				if( curr_key != last_key )
				{
					if( !list_in_values.empty() )
					{
						if(!(last_key.empty() || last_key == str_sp))
						{
							std::list<std::string> list_out_values =::mapreduce::util::reduce(last_key, list_in_values);
							::mapreduce::util::write_to_stream(last_key, list_out_values, ofs );
							list_in_values.clear();
						}
					}

					last_key = curr_key;
				}

				list_in_values.push_back(curr_val);
			}
		}

		if( !list_in_values.empty() )
		{
			if( !(last_key.empty() || last_key == str_sp) )
			{
				std::list<std::string> list_out_values =::mapreduce::util::reduce(last_key, list_in_values);
				::mapreduce::util::write_to_stream(last_key, list_out_values, ofs );
				list_in_values.clear();
			}
		}

		ifs_1.close();
		ifs_2.close();
		ofs.close();

		std::remove(str_out_file_1.data());
		std::remove(str_out_file_2.data());
		//std::rename(str_out_file.data(), str_out_file_1.data());
	}
  }
}

#endif
