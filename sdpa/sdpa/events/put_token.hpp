// bernd.loerwald@itwm.fraunhofer.de

#ifndef SDPA_EVENTS_PUT_TOKEN_HPP
#define SDPA_EVENTS_PUT_TOKEN_HPP

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/Serialization.hpp>

#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <string>
#include <sstream>

namespace sdpa
{
  namespace events
  {
    class put_token : public JobEvent
    {
    public:
      put_token ( address_t from
                , address_t to
                , job_id_t job_id
                , std::string put_token_id
                , std::string place_name
                , pnet::type::value::value_type value
                )
        : JobEvent (from, to, job_id)
        , _put_token_id (put_token_id)
        , _place_name (place_name)
        , _value (value)
      {}

      std::string const& put_token_id() const
      {
        return _put_token_id;
      }
      std::string const& place_name() const
      {
        return _place_name;
      }
      pnet::type::value::value_type const& value() const
      {
        return _value;
      }

      virtual void handleBy (EventHandler* handler) override
      {
        handler->handle_put_token (this);
      }

    private:
      std::string _put_token_id;
      std::string _place_name;
      pnet::type::value::value_type _value;
    };

    SAVE_CONSTRUCT_DATA_DEF (put_token, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->put_token_id());
      SAVE_TO_ARCHIVE (e->place_name());
      std::ostringstream oss;
      oss << pnet::type::value::show (e->value());
      SAVE_TO_ARCHIVE_WITH_TEMPORARY (std::string, oss.str());
    }

    LOAD_CONSTRUCT_DATA_DEF (put_token, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (from, to, job_id);
      LOAD_FROM_ARCHIVE (std::string, put_token_id);
      LOAD_FROM_ARCHIVE (std::string, place_name);
      LOAD_FROM_ARCHIVE (std::string, value);

      ::new (e) put_token ( from
                          , to
                          , job_id
                          , put_token_id
                          , place_name
                          , pnet::type::value::read (value)
                          );
    }

    class put_token_ack : public MgmtEvent
    {
    public:
      put_token_ack (address_t from, address_t to, std::string put_token_id)
        : MgmtEvent (from, to)
        , _put_token_id (put_token_id)
      {}

      std::string const& put_token_id() const
      {
        return _put_token_id;
      }

      virtual void handleBy (EventHandler* handler) override
      {
        handler->handle_put_token_ack (this);
      }

    private:
      std::string _put_token_id;
    };

    SAVE_CONSTRUCT_DATA_DEF (put_token_ack, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->put_token_id());
    }

    LOAD_CONSTRUCT_DATA_DEF (put_token_ack, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA (from, to);
      LOAD_FROM_ARCHIVE (std::string, put_token_id);

      ::new (e) put_token_ack (from, to, put_token_id);
    }
  }
}

#endif