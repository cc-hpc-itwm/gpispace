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

const int KEY_MAX_SIZE = 50;
const std::string delimiters=" \n";
char SPCHAR = '#';

namespace mapreduce
{
  namespace util
  {
    void print_keys(const std::string& msg, const std::vector<std::string>& arr_keys )
    {
      std::ostringstream osstr;
      if( arr_keys.empty() )
      {
        MLOG(INFO, "No border key cached!");
      }

      for( int k=0;k<arr_keys.size(); k++ )
      {
        osstr<<arr_keys[k]<<" ";
      }

      MLOG(INFO, msg<<" "<<osstr.str());
    }

    std::string match_keys(const std::string& keyval_pair, const std::vector<std::string>& arr_border_keys, std::string& matching_pair, int& cid, int& end)
    {
      std::string w;
      bool bMatching = false;

      MLOG(TRACE, "Trying to find the matching pair of the item "<<keyval_pair<<" ...");
      boost::char_separator<char> sep(":");
      boost::tokenizer<boost::char_separator<char> > tok_v(keyval_pair, sep);
      std::vector<std::string> v(4,"");
      v.assign(tok_v.begin(), tok_v.end());

      cid = boost::lexical_cast<long>(v[1]);
      end = boost::lexical_cast<long>(v[2]);

      if( arr_border_keys.size() == 0 )
      {
           MLOG(WARN, "The array of border keys is empty ...");
           matching_pair = "";
           return "";
      }

      for( int k=0;k<arr_border_keys.size() && !bMatching; k++ )
      {
        boost::tokenizer<boost::char_separator<char> > tok_u(arr_border_keys[k], sep);
        std::vector<std::string> u(4, "");
        u.assign(tok_u.begin(), tok_u.end());

        int v1 = cid;
        int u1 = boost::lexical_cast<long>(u[1]);

        int v2 = end;
        int u2 = boost::lexical_cast<long>(u[2]);

        std::ostringstream osstr;

        if( v1 == u1+1 && u2 == v2+1 )
        {
          osstr<<u[3];
          osstr<<v[3];
          w = osstr.str();
          MLOG(INFO, "The recovered word from "<<keyval_pair<<" and "<<arr_border_keys[k]<<" is "<<w);
          bMatching = true;
          matching_pair = arr_border_keys[k];
        }
        else
          if( u1 == v1+1 && v2 == u2+1 )
          {
            osstr<<v[3];
            osstr<<u[3];
            w = osstr.str();
            MLOG(INFO, "The recovered word from "<<keyval_pair<<" and "<<arr_border_keys[k]<<" is "<<w);
            bMatching = true;
            matching_pair = arr_border_keys[k];
          }
      }

      if(w.empty())
        MLOG(INFO, "No matching pair was found!");

      return w;
    }


    bool is_special_item(const std::string& str_item)
    {
      return str_item.find(SPCHAR) != std::string::npos; // true if it begins with #
    }

    bool is_delimiter(char x)
    {
      bool b = (delimiters.find(x) != std::string::npos);
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
      size_t l_pos = l.find(':');
      size_t r_pos = l.find(':');
      std:: string l_pref = l.substr(0, l_pos);
      std:: string r_pref = r.substr(0, r_pos);

      return string_comp(l_pref, r_pref);
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

    template <typename T>
    std::string get_string(typename std::vector<T>& arr)
    {
       std::stringstream sstr;
       for(typename std::vector<T>::iterator it=arr.begin();it!=arr.end();it++)
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
      sstr<<"   border_used = "<<t.border_used<<std::endl;

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
