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
#include <boost/filesystem.hpp>

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

    	  if( list_in_values.size()==1 )
    	  {
    		  list_out_values.push_back(list_in_values.front());
    		  return list_out_values;
    	  }

    	  BOOST_FOREACH(const std::string& str_val, list_in_values)
    	  {
    		  //MLOG(INFO, "Reduce the item "<<str_val);
    		  boost::char_separator<char> sep("[] ");
    		  boost::tokenizer<boost::char_separator<char> > tok(str_val, sep);
    		  for(boost::tokenizer<boost::char_separator<char> >::iterator it = tok.begin(); it != tok.end(); ++it)
    		  {
    			  try {
    				  total += boost::lexical_cast<int>(*it);
    			  }
    			  catch (boost::bad_lexical_cast const &) {
    				  throw std::runtime_error ("could not reduce: key := " + key + " val := " + *it);
    			  }
    		  }
    	  }

    	  // MLOG(INFO, "After reduction, "<<key<<" -> "<<total<<" ");
    	  std::string str_total( boost::lexical_cast<std::string>(total) );
    	  list_out_values.push_back(str_total);

    	  return list_out_values;
      }

      std::string get_reduced_item(std::string& key, std::list<std::string>& list_values )
	  {
		  std::ostringstream oss;
		  oss<<key<<PAIRSEP<<"[";

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

      size_t store(std::string& key, std::list<std::string>& list_values, char* reduce_buff, size_t last_pos, const long& n_max_size)
      {
    	  std::stringstream sstr;

    	  std::string str_pair = get_reduced_item(key, list_values);
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

      void write(std::string& key, std::list<std::string>& list_values, std::ofstream& ofs )
      {
    	  std::string item = get_reduced_item( key, list_values );
		  ofs<<item<<std::endl;
      }

      void reduce_array(const int part_id, const size_t red_slot_size, const std::vector<std::string>& arr_items, char* ptr_shmem, size_t& last_pos )
      {
    	  std::list<std::string> list_in_values;
    	  key_val_pair_t kv_pair, kv_pair_next;

    	  try {
    		  kv_pair = get_key_val(arr_items[0]);
    	  }
		  catch(const std::exception& exc ){
			  throw std::runtime_error("Invalid key-value pair ("+ arr_items[0] + ") :" + +exc.what());
		  }

    	  std::string last_key = kv_pair.first;

    	  last_pos = 0;
    	  for(std::vector<std::string>::const_iterator it=arr_items.begin(); it != arr_items.end(); it++ )
    	  {
    		  try {
    			  kv_pair_next = ::mapreduce::util::get_key_val(*it);
    		  }
    		  catch(const std::exception& exc ){
    			  throw std::runtime_error("Invalid key-value pair ("+ *it + ") :" +exc.what());
    		  }

    		  if( kv_pair_next.first != last_key )
    		  {
    			  std::list<std::string> list_out_values = ::mapreduce::util::reduce(last_key, list_in_values);

    			  try {
    				  last_pos = ::mapreduce::util::store( last_key, list_out_values, ptr_shmem, last_pos, red_slot_size );
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
				last_pos = ::mapreduce::util::store( last_key, list_out_values, ptr_shmem, last_pos, red_slot_size);
			  }
			  catch(const std::exception& exc) {
				  throw std::runtime_error("Reduce slot "+boost::lexical_cast<std::string>(part_id)+":"+exc.what());
			  }
		  }
      }

      void reduce_and_write_array(const std::vector<std::string>& arr_items,  const std::string& str_part_in_file )
      {
    	  std::list<std::string> list_in_values;
    	  key_val_pair_t kv_pair = get_key_val(arr_items[0]);
    	  std::string last_key = kv_pair.first;

    	  std::ofstream ofs( str_part_in_file.data() );

    	  for(std::vector<std::string>::const_iterator it=arr_items.begin(); it != arr_items.end(); it++ )
    	  {
    		  ::mapreduce::util::key_val_pair_t kv_pair_next = ::mapreduce::util::get_key_val(*it);
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
		  key_val_pair_t kv_pair = get_key_val(arr_items[0]);
		  std::string last_key = kv_pair.first;

		  for(std::vector<std::string>::const_iterator it=arr_items.begin(); it != arr_items.end(); it++ )
		  {
			  ::mapreduce::util::key_val_pair_t kv_pair_next = ::mapreduce::util::get_key_val(*it);
			  std::string key = kv_pair_next.first;

			  if( key != last_key )
			  {
				  std::list<std::string> list_out_values = ::mapreduce::util::reduce(last_key, list_in_values);

				  try {
					  std::string str_red_item = get_reduced_item(last_key, list_out_values);
					  arr_reduced_items.push_back(str_red_item);
				  }
				  catch(const std::exception& exc) {
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
				  std::string str_red_item = get_reduced_item(last_key, list_out_values);
				  arr_reduced_items.push_back(str_red_item);
			  }
			  catch(const std::exception& exc) {
				  throw std::runtime_error(exc.what());
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

      void merge_and_reduce_buff_into_file(const std::vector<std::string>& arr_items, const std::string& str_part_in_file)
	  {
		  std::list<std::string> list_in_values;
		  key_val_pair_t kv_pair = get_key_val(arr_items[0]);
		  std::string last_key = kv_pair.first;

		  if( !file_exists(str_part_in_file) )
		  {
			  MLOG(INFO, "Create new file "<<str_part_in_file);
			  reduce_and_write_array(arr_items, str_part_in_file);
			  return;
		  }
		  MLOG(INFO, "The file "<<str_part_in_file<<" already exists on disk!!!");

		  //else
		  if( file_size(str_part_in_file.data()) == 0 )
		  {
			  MLOG(INFO, "The reduce file "<<str_part_in_file<<" exists but has the length 0!");
			  reduce_and_write_array(arr_items, str_part_in_file);
			  return;
		  }

		  std::ifstream ifs;
		  ifs.open( str_part_in_file.data() );
		  std::string str_part_out_file = str_part_in_file + std::string(".new");
		  std::ofstream ofs( str_part_out_file.data() );

		  MLOG(INFO, "Merge and reduce ...");
		  std::vector<std::string>::const_iterator it = arr_items.begin();
		  key_val_pair_t kv_pair_arr = get_key_val(*it);
		  char str_curr_line[256];
		  ifs.getline(str_curr_line, 256);

		  key_val_pair_t kv_pair_line_file = get_key_val(str_curr_line);

		  // the array and the file are already reduced
		  while( it != arr_items.end() && !ifs.eof() )
		  {
			  if( kv_pair_arr.first<kv_pair_line_file.first )
			  {
				  //MLOG(INFO,  "store the element "<<*it);
				  ofs<<*it++<<std::endl;
				  if( it != arr_items.end() )
					  kv_pair_arr = get_key_val(*it);
			  }

			  if( kv_pair_arr.first>kv_pair_line_file.first )
			  {
				  // store the element
				  ofs<<str_curr_line<<std::endl;
				  if(!ifs.eof())
				  {
					 str_curr_line[0]='\0';
					 ifs.getline(str_curr_line, 256);
					 if(strcmp(str_curr_line, "") )
						 kv_pair_line_file = get_key_val(str_curr_line);
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
					  kv_pair_arr = get_key_val(*it);
				  }

				  if(!ifs.eof())
				  {
					  str_curr_line[0] = '\0';
					  ifs.getline(str_curr_line, 256);
					  if(strcmp(str_curr_line, "") )
						  kv_pair_line_file = get_key_val(str_curr_line);
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
