#ifndef WE_TYPE_USER_DATA_HPP
#define WE_TYPE_USER_DATA_HPP

#include <map>
#include <string>
#include <errno.h>

namespace we
{
  namespace type
  {
    class user_data
    {
    public:
      void set_user_job_identification (std::string const &id)
      {
        m_user_job_identification = id;
      }

      std::string const &get_user_job_identification ()
      {
        return m_user_job_identification;
      }

      int get (std::string const &key, std::string & value) const
      {
        typedef std::map<std::string, std::string> map_type_t;
        map_type_t::const_iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          return -ENOKEY;
        }
        else
        {
          value = it->second;
          return 0;
        }
      }

      int put (std::string const &key, std::string const &value)
      {
        typedef std::map<std::string, std::string> map_type_t;
        m_values.insert (map_type_t::value_type (key, value));
        return 0;
      }

      int del (std::string const &key)
      {
        typedef std::map<std::string, std::string> map_type_t;
        map_type_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          return -ENOKEY;
        }
        else
        {
          m_values.erase (it);
          return 0;
        }
      }
    private:
      std::string m_user_job_identification;
      std::map<std::string, std::string> m_values;
    };
  }
}

#endif
