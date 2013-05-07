
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <util/reduce.hpp>
#include <fhglog/fhglog.hpp>
#include <vector>

const int NTESTKEYS = 5;
using namespace mapreduce::util;

void test1()
{
	std::string arr0[] = {"a", "a", "b", "c", "c", "c", "e", "h", "i"};
	std::string arr1[] = {"a", "b", "c", "e", "e", "g", "h", "i"};

	std::vector<std::string> arr_items_0;
	for(int k=0; k<9; k++)
	{
		key_val_pair_t kvp(arr0[k],"1");
		arr_items_0.push_back(kvpair2str(kvp));
	}

	std::vector<std::string> arr_items_1;
	for(int i=0; i<8; i++)
	{
		key_val_pair_t kvp(arr1[i],"1");
		arr_items_1.push_back(kvpair2str(kvp));
	}

	char* ptr_shmem = new char[1000];
	size_t last_pos = merge_and_reduce_arr_arr_2_buff( 0, 1000000, arr_items_0, arr_items_1, ptr_shmem );
	ptr_shmem[last_pos]=0;

	std::cout<<"The result is: \""<<ptr_shmem<<"\""<<std::endl;
}

void test2()
{
	key_val_pair_t kv_pair("John", "Doe");

	std::string str_pair(kvpair2str(kv_pair));
	std::cout<<"The encoded string is: \""<<str_pair<<"\""<<std::endl;

	key_val_pair_t kv_pair_new = str2kvpair(str_pair);
	std::cout<<"The decoded pair is: <"<<kv_pair_new.first<<","<<kv_pair_new.second<<">"<<std::endl;

	std::cout<<"The key: "<<key(str_pair)<<std::endl;
	std::cout<<"The value: "<<val(str_pair)<<std::endl;

	const char* arr_keys[] = {"#0#2","#0#","#0","2#0#"," 7#0#"};

	for(int k=0; k<NTESTKEYS; k++)
	{
		std::string key(arr_keys[k]);

		if(is_special_key(key))
			std::cout<<"The item "<<key<<" is special!"<<std::endl;
		else
			std::cout<<"The item "<<key<<" is NOT special!"<<std::endl;
	}
}

void test_merge_and_reduce_files_k_way(std::vector<std::string>& arr_file, std::string& str_out_file)
{
	merge_and_reduce_files_k_way(arr_file, str_out_file);
}

void init(std::vector<std::string>& arr_in_file)
{
	arr_in_file.push_back("test_file_0.txt");
	arr_in_file.push_back("test_file_1.txt");
	arr_in_file.push_back("test_file_2.txt");
	arr_in_file.push_back("test_file_3.txt");

	std::ofstream ifs0(arr_in_file[0].data());

	ifs0<<"acku"<<PAIRSEP<<"1"<<std::endl;
	ifs0<<"babacu"<<PAIRSEP<<"1"<<std::endl;
	ifs0<<"cucu"<<PAIRSEP<<"1"<<std::endl;
	ifs0<<"salo"<<PAIRSEP<<"1"<<std::endl;
	ifs0<<"secu"<<PAIRSEP<<"1"<<std::endl;
	ifs0<<"tecku"<<PAIRSEP<<"1"<<std::endl;
	ifs0<<"tzucu"<<PAIRSEP<<"1"<<std::endl;
	ifs0.close();

	std::ofstream ifs1(arr_in_file[1].data());
	ifs1<<"acku"<<PAIRSEP<<"1"<<std::endl;
	ifs1<<"babacu"<<PAIRSEP<<"1"<<std::endl;
	ifs1<<"calacu"<<PAIRSEP<<"1"<<std::endl;
	ifs1<<"cecu"<<PAIRSEP<<"1"<<std::endl;
	ifs1<<"cucu"<<PAIRSEP<<"1"<<std::endl;
	ifs1<<"ralo"<<PAIRSEP<<"1"<<std::endl;
	ifs1<<"techno"<<PAIRSEP<<"1"<<std::endl;
	ifs1<<"tzucu"<<PAIRSEP<<"1"<<std::endl;
	ifs1.close();

	std::ofstream ifs2(arr_in_file[2].data());
	ifs2<<"acku"<<PAIRSEP<<"1"<<std::endl;
	ifs2<<"babacu"<<PAIRSEP<<"1"<<std::endl;
	ifs2<<"calacu"<<PAIRSEP<<"1"<<std::endl;
	ifs2<<"cecu"<<PAIRSEP<<"1"<<std::endl;
	ifs2<<"cucu"<<PAIRSEP<<"1"<<std::endl;
	ifs2<<"ralo"<<PAIRSEP<<"1"<<std::endl;
	ifs2<<"salo"<<PAIRSEP<<"1"<<std::endl;
	ifs2<<"techno"<<PAIRSEP<<"1"<<std::endl;
	ifs2<<"tzucu"<<PAIRSEP<<"1"<<std::endl;
	ifs2.close();

	std::ofstream ifs3(arr_in_file[3].data());
	ifs3<<"calacu"<<PAIRSEP<<"1"<<std::endl;
	ifs3<<"cecu"<<PAIRSEP<<"1"<<std::endl;
	ifs3<<"ralo"<<PAIRSEP<<"1"<<std::endl;
	ifs3<<"techno"<<PAIRSEP<<"1"<<std::endl;
	ifs3<<"tecku"<<PAIRSEP<<"1"<<std::endl;
	ifs3<<"tzucu"<<PAIRSEP<<"1"<<std::endl;
	ifs3.close();
}

void validate(std::vector<std::string>& arr_in_file, std::string& str_out_file)
{
	std::map<std::string, int> map_word_counters;
	std::string word;
	ifstream ifs;

	BOOST_FOREACH(std::string& file, arr_in_file)
	{
		ifs.open(file.data());
		while(!ifs.eof())
		{
			word.clear();
			ifs>>word;
			if(!word.empty())
			{
				std::string k = key(word);
				if( map_word_counters.find(k) == map_word_counters.end() )
					map_word_counters.insert(std::pair<std::string, int>(k, 1));
				else
					map_word_counters[k]++;
			}
		}

		ifs.close();
	}

	ifs.open(str_out_file.data());
	while(!ifs.eof())
	{
		std::string item;
		ifs>>item;
		if(!item.empty())
		{
			key_val_pair_t kv_pair = str2kvpair(item);

			if (map_word_counters[kv_pair.first] == boost::lexical_cast<int>(kv_pair.second) )
				std::cout<<"The result is ok for the word "<<kv_pair.first<<": "<<kv_pair.second<<" occurrences!"<<std::endl;
			else
			{
				std::cout<<"The result is NOT ok for the word "<<kv_pair.first<<": "<<map_word_counters[kv_pair.first]<<" occurrences, but counted "
						 <<kv_pair.second<<"!"<<std::endl;

				throw std::runtime_error("test3 failed!");
			}
		}
	}
}

void test3()
{
	std::string str_out_file("test_out.txt");
	std::vector<std::string> arr_in_file;
	init(arr_in_file);
	test_merge_and_reduce_files_k_way(arr_in_file, str_out_file);

	std::vector<std::string> arr_file;
	init(arr_file);
	validate(arr_file, str_out_file);
}

int main ()
{
	test1();
	test2();
	test3();

	return 0;
}
