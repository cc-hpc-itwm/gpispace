#ifndef FHG_LOG_LOGGER_PATH_HPP
#define FHG_LOG_LOGGER_PATH_HPP 1

#include <fhglog/util.hpp>
#include <vector>
#include <boost/lexical_cast.hpp>

namespace fhg
{
  namespace log
  {
    class logger_path
    {
    public:
      typedef std::vector<std::string> path_type;

      static const std::string & SEPERATOR()
      {
        static std::string s (".");
        return s;
      }

      logger_path ();
      // non-explicit
      logger_path (const std::string & a_path);
      logger_path (const logger_path & a_path);
      logger_path (const path_type & a_path);

      template <typename T>
      logger_path & operator/(T const & t)
      {
        fhg::log::split( boost::lexical_cast<std::string>(t)
                       , SEPERATOR()
                       , std::back_inserter (path_)
                       );
        return *this;
      }

      logger_path & operator/(logger_path const & p)
      {
        std::copy ( p.path_.begin()
                  , p.path_.end()
                  , std::back_inserter(path_)
                  );
        return *this;
      }

      std::string str(void) const;
      void str(const std::string &);

      operator std::string () { return str(); }

      const std::string & operator[] (const std::size_t idx) const
      {
        return path_[idx];
      }

      std::string & operator[] (const std::size_t idx)
      {
        return path_[idx];
      }

      std::size_t size() const
      {
        return path_.size();
      }

      bool empty() const
      {
        return path_.empty();
      }
    private:
      path_type path_;
    };

    inline std::ostream & operator << ( std::ostream & s
                                      , const logger_path & p
                                      )
    {
      s << p.str();
      return s;
    }

    inline std::istream & operator >> ( std::istream & s
                                      , logger_path & p
                                      )
    {
      std::string str;
      s >> str;
      p.str(str);
      return s;
    }

    inline bool operator==(const logger_path & a, const logger_path & b)
    {
      return a.str() == b.str();
    }

    inline bool operator!=(const logger_path & a, const logger_path & b)
    {
      return ! (a == b);
    }

    inline bool operator<(const logger_path & a, const logger_path & b)
    {
      return (a.str() < b.str());
    }
  }
}

#endif
