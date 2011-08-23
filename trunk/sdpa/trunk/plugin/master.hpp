#ifndef DRTS_MASTER_HPP
#define DRTS_MASTER_HPP 1

#include <string>
#include <ctime>

namespace drts
{
  class Master
  {
  public:
    enum state_code
      {
        CONNECTED = 0
      , NOT_CONNECTED
      };

    explicit Master(std::string const& name);
    ~Master () {}

    inline bool is_connected () const
    {
      return m_state == CONNECTED;
    }

    std::string const & name() const { return m_name; }

  private:
    // disallow copy construction
    Master(Master const & other);
    Master & operator=(Master const & other);

    std::string m_name;
    state_code m_state;
    time_t m_last_recv;
    time_t m_last_send;

    size_t m_num_sent;
    size_t m_num_recv;

    bool m_polling;
    time_t m_poll_interval;
  };
}

#endif // DRTS_MASTER_HPP
