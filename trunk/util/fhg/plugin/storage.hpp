#ifndef FHG_PLUGIN_STORAGE_HPP
#define FHG_PLUGIN_STORAGE_HPP 1

#include <string>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

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
      int load (std::string const &key, T &val)
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

      virtual int remove (std::string const &key) = 0;
      virtual int commit () = 0;
      virtual int flush () = 0;
    protected:
      virtual int write (std::string const &key, std::string const &value) = 0;
      virtual int read (std::string const &key, std::string &value) = 0;
    private:
      typedef boost::archive::xml_oarchive oarchive;
      typedef boost::archive::xml_iarchive iarchive;

      template <typename T>
      int decode (std::string const &text, T & v)
      {
        std::istringstream s (text);
        try
        {
          iarchive ar (s);
          ar & BOOST_SERIALIZATION_NVP (v);

          return 0;
        }
        catch (std::exception const &ex)
        {
          return -EINVAL;
        }
      }

      template <typename T>
      int encode (T const & v, std::string & text)
      {
        std::ostringstream s;
        try
        {
          oarchive ar (s);
          ar & BOOST_SERIALIZATION_NVP (v);
          text = s.str();

          return 0;
        }
        catch (std::exception const &ex)
        {
          return -EINVAL;
        }
      }

      bool m_auto_commit;
    };
  }
}

#endif
