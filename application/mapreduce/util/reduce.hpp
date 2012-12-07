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
                {
                   try
                   {
                     total += boost::lexical_cast<int>(v[k]);
                   }
                   catch (boost::bad_lexical_cast const &)
                   {
                     throw std::runtime_error ("could not reduce: key := " + key + " val := " + v[k]);
                   }
                }
        }

       // MLOG(INFO, "After reduction, "<<key<<" -> "<<total<<" ");
        std::string str_total(boost::lexical_cast<std::string>(total));
        list_out_values.push_back(str_total);

        return list_out_values;
      }

      size_t store(std::string& key, std::list<std::string>& list_values, char* reduce_buff, size_t last_pos, const long& n_max_size)
      {
        std::stringstream sstr;
        sstr<<key<<PAIRSEP<<"[";

        for( std::list<std::string>::iterator it=list_values.begin(); it!=list_values.end(); it++ )
        {
          sstr<<*it;

          if( boost::next(it) != list_values.end() )
            sstr<<SPCH;
          else
            sstr<<"]"<<SPCH;
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
			ofs<<key<<PAIRSEP<<"[";

			for( std::list<std::string>::iterator it=list_values.begin(); it!=list_values.end(); it++ )
			{
				ofs<<*it;

				if( boost::next(it) != list_values.end() )
					ofs<<SPCH;
				else
					ofs<<"]"<<SPCH<<std::endl;
			}
		}

		void reduce_array(const int part_id, const size_t red_slot_size, const std::vector<std::string>& arr_items, char* ptr_shmem, size_t& last_pos )
		{
			std::list<std::string> list_in_values;
			key_val_pair_t kv_pair = get_key_val(arr_items[0]);
			std::string last_key = kv_pair.first;

			last_pos = 0;
			for(std::vector<std::string>::const_iterator it=arr_items.begin(); it != arr_items.end(); it++ )
			{
				 ::mapreduce::util::key_val_pair_t kv_pair_next = ::mapreduce::util::get_key_val(*it);

				if( kv_pair_next.first != last_key )
				{
					std::list<std::string> list_out_values = ::mapreduce::util::reduce(last_key, list_in_values);

					try {
						last_pos = ::mapreduce::util::store( last_key, list_out_values, ptr_shmem, last_pos, red_slot_size );
					}
					catch(const std::exception& exc)
					{
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
				catch(const std::exception& exc)
				{
					throw std::runtime_error("Reduce slot "+boost::lexical_cast<std::string>(part_id)+":"+exc.what());
				}
			}
		}

		void reduce_and_write_array(std::ofstream& ofs, const std::vector<std::string>& arr_items )
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
						::mapreduce::util::write(last_key, list_out_values, ofs );
					}
					catch(const std::exception& exc)
					{
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
				catch(const std::exception& exc)
				{
					ofs.close();
					throw std::runtime_error(exc.what());
				}
			}

			ofs.close();
  	  }
  }
}

#endif
