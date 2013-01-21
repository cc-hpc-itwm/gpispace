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
    		  throw std::runtime_error ("could not reduce: key := " + key + ", empty list of values!");
    	  }

    	  BOOST_FOREACH(const std::string& str_item, list_in_values)
    	  {
    		  std::string str_val(str_item);
    		  while( boost::algorithm::starts_with(str_val, "[") && boost::algorithm::ends_with(str_val, "]") )
			  {
				  std::string val = str_val.substr(1, str_val.size()-2);
				  str_val = val;
			  }

    		  boost::char_separator<char> sep(" ");

    		  boost::tokenizer<boost::char_separator<char> > tok(str_val, sep);
    		  for(boost::tokenizer<boost::char_separator<char> >::iterator it = tok.begin(); it != tok.end(); ++it)
    		  {
    			  try {
    				  total += boost::lexical_cast<int>(*it);
    			  }
    			  catch (boost::bad_lexical_cast const &) {
    				  if(boost::algorithm::starts_with(*it, "[") && boost::algorithm::ends_with(*it, "]") )
    				  {
    					  std::list<string> list_in_interm_values;
    					  list_in_interm_values.push_back(*it);
    					  std::list<std::string> list_out_interm_val = reduce(key, list_in_interm_values);
    					  total += boost::lexical_cast<int>(*list_out_interm_val.begin());
    				  }
    				  else
    					  throw std::runtime_error ("could not reduce: key := " + key + " val := " + *it);
    			  }
    		  }
    	  }

    	  // MLOG(INFO, "After reduction, "<<key<<" -> "<<total<<" ");
    	  std::string str_total( boost::lexical_cast<std::string>(total) );
    	  list_out_values.push_back(str_total);

    	  return list_out_values;
      }

      void write(std::string& key, std::list<std::string>& list_values, std::ofstream& ofs )
      {
    	  key_val_pair_t kvp(key, list2str(list_values));
    	  std::string str_pair = kvpair2str(kvp);
		  ofs<<str_pair<<std::endl;
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

      // assume that the both input arrays are sorted reduced !!!!
      void merge_and_reduce_arr_buff(const int part_id, const size_t red_slot_size, const std::vector<std::string>& arr_items_1, const std::vector<std::string>& arr_items_2, char* ptr_shmem, size_t& last_pos )
   	  {
   		  std::list<std::string> list_in_values;

   		  MLOG(INFO, "Merge and reduce ...");
   		  std::vector<std::string>::const_iterator it_1 = arr_items_1.begin();
   		  std::vector<std::string>::const_iterator it_2 = arr_items_2.begin();

   		  key_val_pair_t kv_pair_arr_1 = str2kvpair(*it_1);
   		  key_val_pair_t kv_pair_arr_2 = str2kvpair(*it_2);

   		  // the array and the file are already reduced
   		  std::string str_pair;
   		  while( it_1 != arr_items_1.end() && it_2 != arr_items_2.end() )
   		  {
   			  str_pair="";

   			  if( kv_pair_arr_1.first < kv_pair_arr_2.first )
   			  {
   				  str_pair = *it_1++;
   				  if( it_1 != arr_items_1.end() )
   					  kv_pair_arr_1 = str2kvpair(*it_1);
   			  }
   			  else
   			  if( kv_pair_arr_1.first > kv_pair_arr_2.first )
   			  {
   				  str_pair = *it_2++;
   				  if( it_2 != arr_items_2.end() )
   					  kv_pair_arr_2 = str2kvpair(*it_2);
   			  }
   			  else
   			  if( kv_pair_arr_1.first == kv_pair_arr_2.first )
   			  {
   				  std::list<std::string> list_in_values;
   				  list_in_values.push_back(kv_pair_arr_1.second);
   				  list_in_values.push_back(kv_pair_arr_2.second);

   				  if( !list_in_values.empty() )
   				  {
   					  std::list<std::string> list_out_values = ::mapreduce::util::reduce(kv_pair_arr_1.first, list_in_values);
   					  key_val_pair_t kvp(kv_pair_arr_1.first, list2str(list_out_values));
   					  str_pair = kvpair2str(kvp);
   				  }

   				  if( ++it_1 != arr_items_1.end() )
   					  if(!it_1->empty())
   						  kv_pair_arr_1 = str2kvpair(*it_1);

   				  if( ++it_2 != arr_items_2.end() )
   					  if(!it_2->empty())
   						  kv_pair_arr_2 = str2kvpair(*it_2);

   				  if( kv_pair_arr_1.second.empty() || kv_pair_arr_2.second.empty() )
   				  {
   					  MLOG(FATAL, "HoobaBoobaa, empty values!!!!!!");
   					  throw std::runtime_error("Empty values!");
   				  }
   			  }

   			  try {
   				  if(!str_pair.empty())
   					  last_pos = ::mapreduce::util::write_to_buff( str_pair, ptr_shmem, last_pos, red_slot_size );
			  }
			  catch(const std::exception& exc)
			  {
				  throw std::runtime_error(exc.what());
			  }
   		  }

   		  if( it_1 == arr_items_1.end() )
   		  {
   			  while( it_2 != arr_items_2.end() )
   				last_pos = ::mapreduce::util::write_to_buff( *it_2++, ptr_shmem, last_pos, red_slot_size );
   		  }

   		  if( it_2 == arr_items_2.end() )
   		  {
   			  while( it_1 != arr_items_1.end() )
   				  last_pos = ::mapreduce::util::write_to_buff( *it_1++, ptr_shmem, last_pos, red_slot_size );
   		  }
   	  }

  	// assume that the both input arrays are sorted reduced !!!!
	size_t merge_and_reduce_arr_buff_rep( const int part_id,
										const size_t red_slot_size,
										const std::vector<std::string>& arr_items_1,
										const std::vector<std::string>& arr_items_2,
										char* reduce_buff )
	{
		std::list<std::string> list_in_values;
		size_t last_pos = 0;

		MLOG(INFO, "Merge and reduce ...");
		std::vector<std::string>::const_iterator it_1 = arr_items_1.begin();
		std::vector<std::string>::const_iterator it_2 = arr_items_2.begin();

		key_val_pair_t kv_pair_arr_1 = str2kvpair(*it_1);
		key_val_pair_t kv_pair_arr_2 = str2kvpair(*it_2);

		// the array and the file are already reduced
		std::string str_pair, curr_item;
		std::string curr_key, last_key, curr_val;

		while( it_1 != arr_items_1.end() || it_2 != arr_items_2.end() )
		{
			if( it_1 != arr_items_1.end() && it_2 != arr_items_2.end() )
				curr_item = (key(*it_1)<key(*it_2))?*it_1++:*it_2++;
			else
				curr_item = (it_2!=arr_items_2.end())?*it_2++:*it_1++;

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


      void reduce_arr_file(const std::vector<std::string>& arr_items,  const std::string& str_part_in_file )
      {
    	  std::list<std::string> list_in_values;
    	  key_val_pair_t kv_pair = str2kvpair(arr_items[0]);
    	  std::string last_key = kv_pair.first;

    	  std::ofstream ofs( str_part_in_file.data() );

    	  for(std::vector<std::string>::const_iterator it=arr_items.begin(); it != arr_items.end(); it++ )
    	  {
    		  ::mapreduce::util::key_val_pair_t kv_pair_next = ::mapreduce::util::str2kvpair(*it);
    		  std::string key = kv_pair_next.first;

    		  if( key != last_key )
    		  {
    			  std::list<std::string> list_out_values = ::mapreduce::util::reduce(last_key, list_in_values);

    			  try {
    				  ::mapreduce::util::write(last_key, list_out_values, ofs );
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
    			  ::mapreduce::util::write(last_key, list_out_values, ofs );
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

      void merge_and_reduce_arr_file(const std::vector<std::string>& arr_items, const std::string& str_part_in_file)
	  {
		  std::list<std::string> list_in_values;

		  if( !file_exists(str_part_in_file) )
		  {
			  MLOG(INFO, "Create new file "<<str_part_in_file);
			  reduce_arr_file(arr_items, str_part_in_file);
			  return;
		  }
		  MLOG(INFO, "The file "<<str_part_in_file<<" already exists on disk!!!");

		  //else
		  if( file_size(str_part_in_file.data()) == 0 )
		  {
			  MLOG(INFO, "The reduce file "<<str_part_in_file<<" exists but has the length 0!");
			  reduce_arr_file(arr_items, str_part_in_file);
			  return;
		  }

		  std::ifstream ifs;
		  ifs.open( str_part_in_file.data() );
		  std::string str_part_out_file = str_part_in_file + std::string(".new");
		  std::ofstream ofs( str_part_out_file.data() );

		  MLOG(INFO, "Merge and reduce ...");
		  std::vector<std::string>::const_iterator it = arr_items.begin();
		  key_val_pair_t kv_pair_arr = str2kvpair(*it);
		  char str_curr_line[256];
		  ifs.getline(str_curr_line, 256);

		  key_val_pair_t kv_pair_line_file = str2kvpair(str_curr_line);

		  // the array and the file are already reduced
		  while( it != arr_items.end() && !ifs.eof() )
		  {
			  if( kv_pair_arr.first<kv_pair_line_file.first )
			  {
				  ofs<<*it++<<std::endl;
				  if( it != arr_items.end() )
					  kv_pair_arr = str2kvpair(*it);
			  }

			  if( kv_pair_arr.first>kv_pair_line_file.first )
			  {
				  // write_to_buff the element
				  ofs<<str_curr_line<<std::endl;
				  if(!ifs.eof())
				  {
					 str_curr_line[0]='\0';
					 ifs.getline(str_curr_line, 256);
					 if(strcmp(str_curr_line, "") )
						 kv_pair_line_file = str2kvpair(str_curr_line);
				  }
			  }

			  if( kv_pair_arr.first == kv_pair_line_file.first )
			  {
				  std::list<std::string> list_in_values;
				  list_in_values.push_back(kv_pair_arr.second);
				  list_in_values.push_back(kv_pair_line_file.second);

				  if( !list_in_values.empty() )
				  {
					  std::list<std::string> list_out_values = ::mapreduce::util::reduce(kv_pair_arr.first, list_in_values);

					  try {
						  ::mapreduce::util::write(kv_pair_arr.first, list_out_values, ofs );
					  }
					  catch(const std::exception& exc)
					  {
						  ofs.close();
						  throw std::runtime_error(exc.what());
					  }
				  }

				  if( ++it != arr_items.end() )
				  {
					  kv_pair_arr = str2kvpair(*it);
				  }

				  if(!ifs.eof())
				  {
					  str_curr_line[0] = '\0';
					  ifs.getline(str_curr_line, 256);
					  if(strcmp(str_curr_line, "") )
						  kv_pair_line_file = str2kvpair(str_curr_line);
				  }
			  }
		  }

		  if( it == arr_items.end() )
		  {
			  // write the rest of the input file
			  while( !ifs.eof() )
			  {
				  ofs<<str_curr_line<<std::endl;
				  str_curr_line[0] = '\0';
				  ifs.getline(str_curr_line, 256);
			  }
		  }

		  if( ifs.eof() )
		  {
			  // write the rest of the array into the output file
			  while( it != arr_items.end() )
				  ofs<<*it++<<std::endl;
		  }

		  ifs.close();
		  ofs.close();

		  std::remove(str_part_in_file.data());
		  std::rename(str_part_out_file.data(), str_part_in_file.data());
	  }
  }
}

#endif
