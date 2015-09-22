#pragma once

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/Serialization.hpp>

#include <we/type/value.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <exception>
#include <string>
#include <sstream>

namespace sdpa
{
  namespace events
  {
    class workflow_response : public JobEvent
    {
    public:
      workflow_response ( job_id_t job_id
                        , std::string workflow_response_id
                        , std::string place_name
                        , pnet::type::value::value_type value
                        )
        : JobEvent (job_id)
        , _workflow_response_id (workflow_response_id)
        , _place_name (place_name)
        , _value (value)
      {}

      std::string const& workflow_response_id() const
      {
        return _workflow_response_id;
      }
      std::string const& place_name() const
      {
        return _place_name;
      }
      pnet::type::value::value_type const& value() const
      {
        return _value;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handle_workflow_response (source, this);
      }

    private:
      std::string _workflow_response_id;
      std::string _place_name;
      pnet::type::value::value_type _value;
    };

    SAVE_CONSTRUCT_DATA_DEF (workflow_response, e)
    {
      SAVE_JOBEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->workflow_response_id());
      SAVE_TO_ARCHIVE (e->place_name());
      std::ostringstream oss;
      oss << pnet::type::value::show (e->value());
      SAVE_TO_ARCHIVE_WITH_TEMPORARY (std::string, oss.str());
    }

    LOAD_CONSTRUCT_DATA_DEF (workflow_response, e)
    {
      LOAD_JOBEVENT_CONSTRUCT_DATA (job_id);
      LOAD_FROM_ARCHIVE (std::string, workflow_response_id);
      LOAD_FROM_ARCHIVE (std::string, place_name);
      LOAD_FROM_ARCHIVE (std::string, value);

      ::new (e) workflow_response ( job_id
                                  , workflow_response_id
                                  , place_name
                                  , pnet::type::value::read (value)
                                  );
    }

    class workflow_response_response : public MgmtEvent
    {
    public:
      using value_t = pnet::type::value::value_type;
      using content_t = boost::variant<value_t, std::exception_ptr>;

      workflow_response_response ( std::string workflow_response_id
                                 , content_t content
                                 )
        : MgmtEvent()
        , _workflow_response_id (workflow_response_id)
        , _content (std::move (content))
      {}

      std::string const& workflow_response_id() const
      {
        return _workflow_response_id;
      }

      value_t const& get() const
      {
        if (boost::get<value_t> (&_content))
        {
          return boost::get<value_t> (_content);
        }
        else
        {
          std::rethrow_exception (boost::get<std::exception_ptr> (_content));
        }
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handle_workflow_response_response (source, this);
      }

      //! \note for serialization only
      content_t const& content() const
      {
        return _content;
      }

    private:
      std::string _workflow_response_id;
      content_t _content;
    };

    namespace
    {
      using serialized = std::pair<bool, std::string>;
    }

    SAVE_CONSTRUCT_DATA_DEF (workflow_response_response, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->workflow_response_id());

      struct : public boost::static_visitor<std::pair<bool, std::string>>
      {
        std::pair<bool, std::string> operator()
          (workflow_response_response::value_t const& value) const
        {
          return { true
                 , boost::lexical_cast<std::string>
                     (pnet::type::value::show (value))
                 };
        }

        std::pair<bool, std::string> operator()
          (std::exception_ptr const& ex) const
        {
          return { false
                 , fhg::util::serialization::exception::serialize
                     ( ex
                     , fhg::util::serialization::exception::serialization_functions()
                     , fhg::util::serialization::exception::aggregated_serialization_functions()
                     )
                 };
        }
      } visitor;

      SAVE_TO_ARCHIVE_WITH_TEMPORARY
        (serialized, boost::apply_visitor (visitor, e->content()));
    }

    LOAD_CONSTRUCT_DATA_DEF (workflow_response_response, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (std::string, workflow_response_id);

      LOAD_FROM_ARCHIVE (serialized, content);
      if (content.first)
      {
        ::new (e) workflow_response_response
          (workflow_response_id, pnet::type::value::read (content.second));
      }
      else
      {
        ::new (e) workflow_response_response
          ( workflow_response_id
          , fhg::util::serialization::exception::deserialize
              ( content.second
              , fhg::util::serialization::exception::serialization_functions()
              , fhg::util::serialization::exception::aggregated_serialization_functions()
              )
          );
      }
    }
  }
}
