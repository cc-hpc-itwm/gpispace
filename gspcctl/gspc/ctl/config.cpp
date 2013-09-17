#include "config.hpp"
#include "system.hpp"

#include <fstream>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>

#include <fhg/util/join.hpp>

#include <json_spirit_reader_template.h>
#include <json_spirit_writer_template.h>

namespace gspc
{
  namespace ctl
  {
    namespace detail
    {
      class config_category : public boost::system::error_category
      {
      public:
        const char *name () const
        {
          return "gspc.config";
        }

        std::string message (int value) const
        {
          switch (value)
          {
          case error::config_no_section:
            return "key did not contain a section";
          case error::config_no_name:
            return "key did not contain a name";
          case error::config_no_such_key:
            return "key not found";
          case error::config_invalid_key:
            return "invalid key";
          case error::config_invalid:
            return "invalid config";
          default:
            return "gspc.config error";
          }
        }
      };
    }

    inline const boost::system::error_category & get_config_category ()
    {
      static detail::config_category cat;
      return cat;
    }
  }
}

namespace boost
{
  namespace system
  {
    template <> struct is_error_code_enum<gspc::ctl::error::config_errors>
    {
      static const bool value = true;
    };
  }
}

namespace gspc
{
  namespace ctl
  {
    namespace error
    {
      inline boost::system::error_code make_error_code (config_errors e)
      {
        return boost::system::error_code (static_cast<int>(e), get_config_category ());
      }
    }
  }
}

namespace gspc
{
  namespace ctl
  {
    config_t config_read ()
    {
      json_spirit::Object cfg = json_spirit::Object ();

      std::list<json_spirit::Object> objs;
      objs.push_back (config_read_system ().get_obj ());
      objs.push_back (config_read_site ().get_obj ());
      objs.push_back (config_read_user ().get_obj ());

      while (not objs.empty ())
      {
        json_spirit::Object obj = objs.front (); objs.pop_front ();
        json_spirit::Object::const_iterator it = obj.begin ();
        const json_spirit::Object::const_iterator end = obj.end ();

        while (it != end)
        {
          cfg.push_back (*it);
          ++it;
        }
      }

      return cfg;
    }

    config_t config_read_site ()
    {
      try
      {
        return config_read (site_config_file ());
      }
      catch (std::runtime_error const &)
      {
        return json_spirit::Object ();
      }
    }

    config_t config_read_system ()
    {
      try
      {
        return config_read (system_config_file ());
      }
      catch (std::runtime_error const &)
      {
        return json_spirit::Object ();
      }
    }

    config_t config_read_user ()
    {
      try
      {
        return config_read (user_config_file ());
      }
      catch (std::runtime_error const &)
      {
        return json_spirit::Object ();
      }
    }

    config_t config_read (std::istream &is)
    {
      config_t cfg;
      const bool success = json_spirit::read_stream (is, cfg);
      if (not success)
      {
        throw std::runtime_error ("could not parse");
      }
      return cfg;
    }

    config_t config_read (std::string const &file)
    {
      std::ifstream is (file.c_str ());
      try
      {
        return config_read (is);
      }
      catch (std::runtime_error const &e)
      {
        throw std::runtime_error (std::string (e.what ()) + ": " + file);
      }
    }

    config_t config_read_safe (std::string const &file)
    {
      std::ifstream is (file.c_str ());
      try
      {
        return config_read (is);
      }
      catch (std::runtime_error const &e)
      {
        return json_spirit::Object ();
      }
    }

    void config_write (config_t const &cfg, std::ostream & os)
    {
      json_spirit::write_stream (cfg, os, json_spirit::pretty_print);
    }

    void config_write (config_t const &cfg, std::string const &fname)
    {
      std::ofstream os (fname.c_str ());
      config_write (cfg, os);
    }

    static int s_split_key ( std::string const &key
                           , std::vector<std::string> &path
                           )
    {
      if (key.find ("=") != std::string::npos)
        return error::config_invalid_key;

      std::string::size_type left = key.find (".");
      std::string::size_type right = key.rfind (".");

      if (left == 0)
      {
        return error::config_no_section;
      }
      if (left == std::string::npos || right == (key.size ()-1))
      {
        return error::config_no_name;
      }

      path.push_back (key.substr (0, left));
      if (left - right > 0)
        path.push_back (key.substr (left+1, right - left - 1));
      path.push_back (key.substr (right+1));

      return 0;
    }

    template <typename Visitor>
    static void s_depth_first_traverse ( json_spirit::Value const &val
                                       , std::vector<std::string> trace
                                       , Visitor v
                                       )
    {
      if (val.is_null ())
        return;

      if (val.type () == json_spirit::obj_type)
      {
        json_spirit::Object const & next = val.get_obj ();
        json_spirit::Object::const_iterator it = next.begin ();
        const json_spirit::Object::const_iterator end = next.end ();

        while (it != end)
        {
          std::vector<std::string> new_trace (trace);
          new_trace.push_back (it->name_);

          s_depth_first_traverse (it->value_, new_trace, v);

          ++it;
        }
      }
      else if (val.type () == json_spirit::array_type)
      {
        json_spirit::Array const & next = val.get_array ();
        json_spirit::Array::const_iterator it = next.begin ();
        const json_spirit::Array::const_iterator end = next.end ();

        while (it != end)
        {
          s_depth_first_traverse (*it, trace, v);
          ++it;
        }
      }
      else if (val.type () == json_spirit::str_type)
      {
        v (trace, val.get_str ());
      }
      else if (val.type () == json_spirit::bool_type)
      {
        v (trace, boost::lexical_cast<std::string>(val.get_bool ()));
      }
      else if (val.type () == json_spirit::int_type)
      {
        v (trace, boost::lexical_cast<std::string>(val.get_int ()));
      }
      else if (val.type () == json_spirit::real_type)
      {
        v (trace, boost::lexical_cast<std::string>(val.get_real ()));
      }
      else
      {
        throw std::runtime_error
          ("s_depth_first_traverse: internal error: unknown json type");
      }
    }

    template <typename V>
    static void s_config_traverse (json_spirit::Value const &val, V v)
    {
      std::vector<std::string> trace;
      return s_depth_first_traverse (val, trace, v);
    }

    static std::string s_flatten_visitor ( std::vector<std::pair<std::string, std::string> >& l
                                         , std::vector<std::string> const &trace
                                         , std::string const & value
                                         )
    {
      const std::string key
        (fhg::util::join (trace.begin (), trace.end (), "."));

      l.push_back (std::make_pair (key, value));

      return value;
    }

    static void s_flatten ( json_spirit::Value const &val
                          , std::vector<std::pair<std::string, std::string> >& l
                          )
    {
      s_config_traverse (val, boost::bind (s_flatten_visitor, boost::ref (l), _1, _2));
    }

    std::vector<std::pair<std::string, std::string> >
    config_list (config_t const &cfg)
    {
      std::vector<std::pair<std::string, std::string> > list;
      s_config_traverse (cfg, boost::bind (s_flatten_visitor, boost::ref (list), _1, _2));
      return list;
    }

    std::vector<std::string> config_get_all ( config_t const &cfg
                                            , std::string const &key
                                            , std::string const &value_regex
                                            )
    {
      std::vector<std::string> result;

      typedef std::vector<std::pair<std::string, std::string> > flat_list_t;
      flat_list_t flat_list;
      s_flatten (cfg, flat_list);

      const boost::regex regex (value_regex);

      for (flat_list_t::const_iterator it = flat_list.begin () ; it != flat_list.end () ; ++it)
      {
        if (it->first == key)
        {
          if ( value_regex.empty ()
             || boost::regex_search (it->second, regex)
             )
            result.push_back (it->second);
        }
      }

      return result;
    }

    void config_add ( config_t &cfg
                    , std::string const &key, std::string const& val
                    )
    {
      std::vector<std::string> path;
      int rc = s_split_key (key, path);
      if (rc)
      {
        throw boost::system::system_error
          (error::make_error_code ((error::config_errors)rc));
      }

      if (cfg.type () != json_spirit::obj_type)
        throw boost::system::system_error
          (error::make_error_code (error::config_invalid));

      json_spirit::Object * cur = &cfg.get_obj ();
      while (path.size () > 1)
      {
        const std::string name = path.front (); path.erase (path.begin ());

        json_spirit::Object::iterator it = cur->begin ();
        const json_spirit::Object::iterator end = cur->end ();
        while (it != end)
        {
          if (it->name_ == name)
          {
            break;
          }
          ++it;
        }
        if (it == end)
        {
          cur->push_back (json_spirit::Pair (name, json_spirit::Object ()));
          cur = &cur->back ().value_.get_obj ();
        }
        else
        {
          cur = &it->value_.get_obj ();
        }
      }

      if (path.size () > 1)
      {
        throw boost::system::system_error
          (error::make_error_code (error::config_invalid));
      }

      cur->push_back
        (json_spirit::Pair (path.front (), json_spirit::Value (val)));
    }

    static void config_unset ( json_spirit::Object &obj
                             , std::vector<std::string> path
                             , const boost::regex & regex
                             )
    {
      if (path.empty ())
        return;

      const std::string name = path.front (); path.erase (path.begin ());

      json_spirit::Object::iterator it = obj.begin ();
      const json_spirit::Object::iterator end = obj.end ();

      while (it != end)
      {
        if (it->name_ == name)
        {
          if (path.empty ()) // delete leaf
          {
            if (boost::regex_search (it->value_.get_str (), regex))
            {
              it = obj.erase (it);
            }
            else
            {
              ++it;
            }
          }
          else
          {
            config_unset (it->value_.get_obj (), path, regex);
            if (it->value_.get_obj ().empty ())
            {
              it = obj.erase (it);
            }
            else
            {
              ++it;
            }
          }
        }
        else
        {
          ++it;
        }
      }
    }

    void config_unset ( config_t &cfg
                      , std::string const &key
                      , std::string const &value_regex
                      )
    {
      std::vector<std::string> path;
      int rc = s_split_key (key, path);
      if (rc)
      {
        throw boost::system::system_error
          (error::make_error_code ((error::config_errors)rc));
      }
      const boost::regex regex (value_regex);

      config_unset (cfg.get_obj (), path, regex);
    }

    static void config_replace ( json_spirit::Object &obj
                               , std::vector<std::string> path
                               , std::string const &val
                               , const boost::regex & regex
                               )
    {
      if (path.empty ())
        return;

      const std::string name = path.front (); path.erase (path.begin ());

      json_spirit::Object::iterator it = obj.begin ();
      const json_spirit::Object::iterator end = obj.end ();

      while (it != end)
      {
        if (it->name_ == name)
        {
          if (path.empty ()) // replace leaf
          {
            if (boost::regex_search (it->value_.get_str (), regex))
            {
              it->value_ = val;
            }
          }
          else
          {
            config_replace (it->value_.get_obj (), path, val, regex);
          }
        }

        ++it;
      }
    }

    void config_replace ( config_t &cfg
                        , std::string const &key, std::string const& val
                        , std::string const &value_regex
                        )
    {
      std::vector<std::string> path;
      int rc = s_split_key (key, path);
      if (rc)
      {
        throw boost::system::system_error
          (error::make_error_code ((error::config_errors)rc));
      }
      const boost::regex regex (value_regex);

      config_replace (cfg.get_obj (), path, val, regex);
    }
  }
}
