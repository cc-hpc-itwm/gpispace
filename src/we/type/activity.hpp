#pragma once

#include <we/type/activity.fwd.hpp>

#include <we/eureka_response.hpp>
#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/plugin/Plugins.hpp>
#include <we/type/eureka.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/transition.hpp>
#include <we/type/value/serialize.hpp>
#include <we/workflow_response.hpp>

#include <drts/worker/context_fwd.hpp>

#include <sdpa/requirements_and_preferences.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/access.hpp>

#include <iosfwd>
#include <random>
#include <string>
#include <vector>

namespace gpi
{
  namespace pc
  {
    namespace client
    {
      class api_t;
    }
  }
}

namespace gspc
{
  class scoped_allocation;
}

namespace we
{
  namespace type
  {
    struct TESTING_ONLY{};

    class activity_t
    {
    public:
      explicit activity_t () = default;
      explicit activity_t (we::type::transition_t);

      explicit activity_t (const boost::filesystem::path&);
      explicit activity_t (std::istream&);
      explicit activity_t (const std::string&);

      std::string to_string() const;

      const we::type::transition_t& transition() const;
      boost::optional<eureka_id_type> const& eureka_id() const;

      void set_wait_for_output();
      void put_token
        (std::string place_name, pnet::type::value::value_type const&);
      void inject ( activity_t const&
                  , workflow_response_callback
                  , eureka_response_callback
                  );
      boost::optional<activity_t>
        extract ( std::mt19937&
                , workflow_response_callback const&
                , eureka_response_callback const&
                , gspc::we::plugin::Plugins&
                , gspc::we::plugin::PutToken
                );

      activity_t wrap() const;
      activity_t unwrap() const;

      const TokensOnPorts& input() const;
      void add_input
        ( std::string const& port_name
        , pnet::type::value::value_type const&
        );

      TokensOnPorts output() const;

      bool wait_for_output() const;

      void execute
        ( we::loader::loader&
        , gpi::pc::client::api_t /*const*/ *
        , gspc::scoped_allocation /* const */ *
        , boost::optional<std::string> target_implementation
        , drts::worker::context*
        );

      Requirements_and_preferences requirements_and_preferences
        (gpi::pc::client::api_t*) const;

      explicit activity_t
        ( TESTING_ONLY
        , we::type::transition_t
        , we::transition_id_type
        );

      void add_output_TESTING_ONLY
        ( std::string const& port_name
        , pnet::type::value::value_type const&
        );
      std::list<we::type::preference_t> const preferences_TESTING_ONLY() const;

    private:
      we::type::transition_t _transition;
      boost::optional<we::transition_id_type> _transition_id;

      friend class net_type;
      TokensOnPorts _input;
      TokensOnPorts _output;

      mutable bool _evaluation_context_requested {false};

      explicit activity_t
        ( we::type::transition_t
        , boost::optional<we::transition_id_type>
        );

      friend class boost::serialization::access;
      template<class Archive>
        void serialize (Archive&, const unsigned int);

      void add_input
        ( we::port_id_type const&
        , pnet::type::value::value_type const&
        );
      void add_output
        ( we::port_id_type const&
        , pnet::type::value::value_type const&
        );
      void add_output (expr::eval::context const&);

      //! \note context contains references to input
      expr::eval::context evaluation_context() const;
    };
  }
}

#include <we/type/activity.ipp>
