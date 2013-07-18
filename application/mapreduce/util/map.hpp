// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_MAP
#define _H_MAPREDUCE_UTIL_MAP 1

#include <string>
#include <list>
#include <util/types.hpp>
#include "helper.hpp"
#include "partition.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find_iterator.hpp>


using namespace std;

namespace mapreduce
{
  namespace util
  {
    list_key_val_pairs_t map(const std::string& key, const std::string& val)
    {
       list_key_val_pairs_t list_key_val_pairs;
       list_key_val_pairs.push_back(key_val_pair_t(key,"1"));
       return list_key_val_pairs;
    }

    void map_first_item(int n_part,
						int chunk_id,
						bool first_char_is_delim,
						const std::string& first_item,
						std::vector<set_of_mapped_items_t>& arr_part_buffs )
    {
    	long part_id;
    	std::string str_pair("");

    	if( chunk_id > 0 )
		{
			std::string spec_left_prefix = make_spec_left_prefix(chunk_id);
			std::string key = spec_left_prefix;
			part_id  = hash(key, n_part);

			// the first char of the chunk is a separator/delimiter
			if( first_char_is_delim )
			{
				key = spec_left_prefix;
				str_pair = kvpair2str( key_val_pair_t(key, "" ));
				arr_part_buffs[part_id].push_back(str_pair);
			}
			else
			{
				str_pair = kvpair2str( key_val_pair_t( key, first_item));
				arr_part_buffs[part_id].push_back(str_pair);
			}
		}

    	if( first_char_is_delim  )
		{
			list_key_val_pairs_t list_key_val_pairs = ::mapreduce::util::map(first_item, "");

			BOOST_FOREACH(key_val_pair_t& key_val_pair, list_key_val_pairs)
			{
				str_pair = kvpair2str( key_val_pair_t(key_val_pair.first, key_val_pair.second));
				part_id  = hash( key_val_pair.first, n_part);
				arr_part_buffs[part_id].push_back(str_pair);
			}
		}
    }

    void map_last_item( int n_part,
   						int num_chunks,
   						int chunk_id,
   						bool last_char_is_delim,
   						const std::string& last_item,
   						std::vector<set_of_mapped_items_t>& arr_part_buffs )
    {

    	long part_id;
    	std::string str_pair("");

		if( chunk_id < num_chunks - 1 )
		{
			std::string spec_right_prefix = make_spec_right_prefix(chunk_id);
			std::string key = spec_right_prefix;
			part_id = hash(key, n_part);

			// the last character of the chunk is a delimiter
			if( last_char_is_delim )
			{
				str_pair = kvpair2str( key_val_pair_t( key, ""));
				arr_part_buffs[part_id].push_back(str_pair);
			}
			else
			{
				str_pair = kvpair2str( key_val_pair_t( key, last_item ));
				arr_part_buffs[part_id].push_back(str_pair);
			}
		}

		if( last_char_is_delim )
		{
			list_key_val_pairs_t list_key_val_pairs = ::mapreduce::util::map(last_item, "");

			BOOST_FOREACH(key_val_pair_t& key_val_pair, list_key_val_pairs)
			{
				str_pair = kvpair2str( key_val_pair_t(key_val_pair.first, key_val_pair.second));
				part_id  = hash( key_val_pair.first, n_part);
				arr_part_buffs[part_id].push_back(str_pair);
			}
		}
    }

    // should parallelize this part
	void map_items( int n_part,
					std::vector<std::string>::iterator& tok_begin,
					std::vector<std::string>::iterator& tok_end,
					std::vector<set_of_mapped_items_t>& arr_part_buffs )
	{
		long part_id;
		std::string str_pair("");

		// map the rest of the tokens
		for( std::vector<std::string>::iterator it = tok_begin; it != tok_end; ++it )
		{
			// call here the map function
			list_key_val_pairs_t list_key_val_pairs = ::mapreduce::util::map(*it, "");

			BOOST_FOREACH(key_val_pair_t& key_val_pair, list_key_val_pairs)
			{
				std::string str_pair = kvpair2str( key_val_pair_t(key_val_pair.first, key_val_pair.second));
				part_id  = hash( key_val_pair.first, n_part);
				arr_part_buffs[part_id].push_back(str_pair);
			}
		}
	}

// should parallelize this part
    void map_all_items_mt(   int n_part,
         				 	 int num_chunks,
         				 	 int chunk_id,
         				 	 long map_slot_size,
         				 	 bool first_char_is_delim,
         				 	 bool last_char_is_delim,
         				 	 std::deque<long>& arr_part_ids,
         				 	 std::deque<long>& arr_part_offset,
         				 	 std::deque<long>& arr_part_used,
         				 	 int& counter,
         				 	 size_t& last_pos,
         				 	 char* ptr_shmem)
       {
    	   // try this here: it should be faster!
    	size_t offset = 0;

    	std::vector<set_of_mapped_items_t> arr_part_buffs_0(n_part), arr_part_buffs_1(n_part);

    	std::vector<std::string> arr_items;
    	//arr_items = ::mapreduce::util::get_list_items(ptr_shmem, EXTERNAL_DELIMITERS);

    	std::string str_buff(ptr_shmem);
    	boost::trim_if(str_buff, boost::is_any_of(EXTERNAL_DELIMITERS));
    	boost::split(arr_items, str_buff, boost::is_any_of(EXTERNAL_DELIMITERS), boost::token_compress_on );

    	std::vector<std::string>::iterator tok_first(boost::next(arr_items.begin()));
    	std::vector<std::string>::iterator tok_last(boost::prior(arr_items.end()));

    	std::string first_item(*arr_items.begin());
    	std::string last_item(*tok_last);

    	::mapreduce::util::map_first_item(n_part, chunk_id, first_char_is_delim, first_item, arr_part_buffs_0 );
		::mapreduce::util::map_last_item(n_part, num_chunks, chunk_id, last_char_is_delim, last_item, arr_part_buffs_0 );

		boost::thread_group tg;
		int d=(tok_last-tok_first+1)/2;
		set_of_mapped_items_t::iterator tok_middle_1(boost::next(tok_first, d));
		tg.create_thread( boost::bind(::mapreduce::util::map_items, n_part, tok_first, tok_middle_1, boost::ref(arr_part_buffs_0) ));
		tg.create_thread( boost::bind(::mapreduce::util::map_items, n_part, tok_middle_1, tok_last, boost::ref(arr_part_buffs_1)));
		tg.join_all();

		arr_items.clear();

		for(int part_id = 0; part_id<n_part; part_id++ )
		{
			if(!arr_part_buffs_0[part_id].empty() || !arr_part_buffs_1[part_id].empty())
			{
				offset = last_pos;
				arr_part_ids.push_front(static_cast<long>(part_id));
				arr_part_offset.push_front(static_cast<long>(offset));

				::mapreduce::util::write_arr_to_buff( arr_part_buffs_0[part_id], ptr_shmem, last_pos, map_slot_size );
				arr_part_buffs_0[part_id].clear();

				::mapreduce::util::write_arr_to_buff( arr_part_buffs_1[part_id], ptr_shmem, last_pos, map_slot_size );
				arr_part_buffs_1[part_id].clear();

				arr_part_used.push_front(static_cast<long>(last_pos-offset));

				counter++;
			}
		}
     }
  }
}

#endif
