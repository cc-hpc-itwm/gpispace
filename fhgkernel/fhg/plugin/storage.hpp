#ifndef FHG_PLUGIN_STORAGE_HPP
#define FHG_PLUGIN_STORAGE_HPP 1

#include <errno.h>

#include <string>
#include <sstream>

//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <boost/serialization/variant.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>

namespace fhg
{
  namespace plugin
  {
    class Storage
    {
    public:
      Storage ()
        : m_auto_commit (true)
      {}

      virtual ~Storage() {}

      int save (std::string const &key, const char *val)
      {
        return save (key, std::string (val));
      }

      template <typename T>
      int save (std::string const &key, T const &val)
      {
        std::string encoded;
        int ec;

        ec = encode (val, encoded);
        if (ec == 0)
        {
          return write (key, encoded);
        }
        else
        {
          return ec;
        }
      }

      template <typename T>
      int load (std::string const &key, T &val) const
      {
        std::string encoded;
        int ec;

        ec = read(key, encoded);
        if (ec == 0)
        {
          return decode (encoded, val);
        }
        else
        {
          return ec;
        }
      }

      bool is_auto_commit () const { return m_auto_commit; }
      void set_auto_commit (bool b) { m_auto_commit = b; }

      virtual int add_storage (std::string const &key) = 0;
      virtual Storage *get_storage (std::string const &key) const = 0;
      virtual int del_storage (std::string const &key) = 0;

      virtual int remove (std::string const &key) = 0;
      virtual int commit () = 0;
      virtual int flush () = 0;

      static bool validate (std::string const & key)
      {
        if (key.empty()) return false;
        if (key.find("/") != std::string::npos) return false;
        if (key.find(" ") != std::string::npos) return false;
        if (key.find(".") != std::string::npos) return false;
        return true;
      }

    protected:
      virtual int write (std::string const &key, std::string const &value) = 0;
      virtual int read (std::string const &key, std::string &value) const = 0;
    private:
      typedef boost::archive::xml_oarchive oarchive;
      typedef boost::archive::xml_iarchive iarchive;

      template <typename T>
      static int decode (std::string const &text, T & v)
      {
        std::istringstream s (text);
        try
        {
          iarchive ar (s);
          ar & BOOST_SERIALIZATION_NVP (v);

          return 0;
        }
        catch (std::exception const &)
        {
          return -EINVAL;
        }
      }

      template <typename T>
      static int encode (T const & v, std::string & text)
      {
        std::ostringstream s;
        try
        {
          oarchive ar (s);
          ar & BOOST_SERIALIZATION_NVP (v);
          text = s.str();

          return 0;
        }
        catch (std::exception const &)
        {
          return -EINVAL;
        }
      }

      bool m_auto_commit;
    };
  }
}

#endif
