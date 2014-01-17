#ifndef DRTS_MASTER_HPP
#define DRTS_MASTER_HPP 1

#include <string>

#include <boost/utility.hpp>

namespace drts
{
  class Master : boost::noncopyable
  {
  public:
    explicit Master(std::string const& name);

    inline bool is_connected () const
    {
      return _is_connected;
    }

    inline void is_connected (bool b)
    {
      _is_connected = b;
    }

    std::string const & name() const { return m_name; }

  private:
    std::string m_name;
    bool _is_connected;
  };
}

#endif // DRTS_MASTER_HPP
