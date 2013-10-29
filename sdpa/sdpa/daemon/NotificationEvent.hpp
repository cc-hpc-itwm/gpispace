// alexander.petry@itwm.fraunhofer.de, bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_DAEMON_NOTIFICATION_EVENT_HPP
#define SDPA_DAEMON_NOTIFICATION_EVENT_HPP

#include <plugins/wfe.hpp>

#include <boost/serialization/utility.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/type/net.hpp> // recursive wrapper of transition_t fails otherwise.
#include <we/mgmt/type/activity.hpp>

#include <sstream>
#include <string>

namespace sdpa
{
  namespace daemon
  {
    class NotificationEvent
    {
    public:
      enum state_t
      {
        STATE_STARTED
      , STATE_FINISHED
      , STATE_FAILED
      , STATE_CANCELLED
      , STATE_MAX = STATE_CANCELLED
      };

      NotificationEvent
        ( const std::list<std::string>& sources
        , const std::string &activity_id
        , const state_t &activity_state
        , const we::mgmt::type::activity_t& activity
        , const wfe::meta_data_t& meta_data = wfe::meta_data_t()
        )
          : _components (sources)
          , a_id_(activity_id)
          , a_name_ (activity.nice_name())
          , a_state_(activity_state)
          , _meta_data (meta_data)
      {}

      NotificationEvent (const std::string encoded)
      {
        std::istringstream stream (encoded);
        boost::archive::text_iarchive archive (stream);
        archive & _components;
        archive & a_id_;
        archive & a_name_;
        archive & a_state_;
        archive & _meta_data;
      }
      std::string encoded() const
      {
        std::ostringstream stream;
        boost::archive::text_oarchive archive (stream);
        archive & _components;
        archive & a_id_;
        archive & a_name_;
        archive & a_state_;
        archive & _meta_data;
        return stream.str();
      }

      const std::list<std::string>& components() const { return _components; }
      const std::string &activity_id() const   { return a_id_; }
      const std::string &activity_name() const { return a_name_; }
      const state_t &activity_state() const      { return a_state_; }
      const wfe::meta_data_t& meta_data() const { return _meta_data; }

    private:
      std::list<std::string> _components;
      std::string a_id_;
      std::string a_name_;
      state_t     a_state_;
      wfe::meta_data_t _meta_data;
    };
  }
}

#endif
