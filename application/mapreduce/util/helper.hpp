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

namespace mapreduce
{
  namespace util
  {
    bool comp (const std::string& l, const std::string& r)
    {
      size_t l_pos = l.find(':');
      size_t r_pos = l.find(':');
      std:: string l_pref = l.substr(0, l_pos);
      std:: string r_pref = r.substr(0, r_pos);

      /*std::string resp(l_pref<r_pref?"l_pref<r_pref":"l_pref>=r_pref");
      std::cout<<" l_pref="<<l_pref<<", r_pref="<<r_pref<<resp<<std::endl;*/

      return (l_pref<r_pref);
    }

    std::vector<int> get_array(const std::string& str_input)
    {
      boost::char_separator<char> sep(" \n");
      boost::tokenizer<boost::char_separator<char> > tok(str_input, sep);
      std::vector<std::string> v;
      v.assign(tok.begin(),tok.end());

      std::vector<int> vint;

      BOOST_FOREACH(std::string& strval, v)
        vint.push_back(boost::lexical_cast<int>(strval));

      return vint;
    }

    std::string get_string(std::vector<int>& arr)
    {
       std::stringstream sstr;
       for(std::vector<int>::iterator it=arr.begin();it!=arr.end();it++)
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
      boost::char_separator<char> sep(" \n");
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

      MLOG(INFO, sstr.str());
    }

    long ceil(long a, long b)
    {
      bool c = (a%b);
      return a/b+c;
    }
  }
}

#endif
