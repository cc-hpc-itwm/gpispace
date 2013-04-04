
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
	std::string arr0[] = {"a@1", "a@1", "b@1", "c@1", "c@1", "c@1", "e@1", "h@1", "i@1"};
	std::string arr1[] = {"a@1", "b@1", "c@1", "e@1", "e@1", "g@1", "h@1", "i@1"};

	std::vector<std::string> arr_items_0;
	for(int k=0; k<9; k++)
		arr_items_0.push_back(arr0[k]);

	std::vector<std::string> arr_items_1;
	for(int i=0; i<8; i++)
		arr_items_1.push_back(arr1[i]);

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

	char* arr_keys[] = {"#0#2","#0#","#0","2#0#"," 7#0#"};

	for(int k=0; k<NTESTKEYS; k++)
	{
		std::string key(arr_keys[k]);

		if(is_special_key(key))
			std::cout<<"The item "<<key<<" is special!"<<std::endl;
		else
			std::cout<<"The item "<<key<<" is NOT special!"<<std::endl;
	}
}

int main ()
{
	//test1();
	test2();

	return 0;
}
