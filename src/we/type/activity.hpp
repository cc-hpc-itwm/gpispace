// {petry,rahn}@itwm.fhg.de

#pragma once

#include <we/type/activity.fwd.hpp>

#include <we/type/id.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/transition.hpp>
#include <we/plugin/Plugins.hpp>

#include <we/workflow_response.hpp>
#include <we/eureka_response.hpp>

#include <we/type/eureka.hpp>
#include <we/type/value.hpp>
#include <we/type/value/serialize.hpp>

#include <we/expr/eval/context.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <vector>

#include <iosfwd>
#include <random>
#include <string>

namespace we
{
    namespace type
    {
      class activity_t
      {
      public:
        typedef std::pair< pnet::type::value::value_type
                         , we::port_id_type
                         > token_on_port_t;
        typedef std::vector<token_on_port_t> token_on_port_list_t;
        typedef token_on_port_list_t input_t;
        typedef token_on_port_list_t output_t;

      public:
        explicit activity_t () = default;
        explicit activity_t
          ( const we::type::transition_t&
          , boost::optional<we::transition_id_type> const&
          );

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

        const input_t& input() const;
        void add_input
          ( we::port_id_type const&
          , pnet::type::value::value_type const&
          );

        output_t output() const;
        void add_output
          ( we::port_id_type const&
          , pnet::type::value::value_type const&
          );
        void add_output (expr::eval::context const&);

        bool wait_for_output() const;

        boost::optional<we::transition_id_type> const&
          transition_id() const;

        //! \note context contains references to input
        expr::eval::context evaluation_context() const;

        schedule_data get_schedule_data() const;

        unsigned long memory_buffer_size_total() const
        {
          return !_transition.module_call()
            ? 0UL
            : _transition.module_call()
              ->memory_buffer_size_total (evaluation_context())
            ;
        }

        std::list<we::type::requirement_t> const requirements() const;
        std::list<we::type::preference_t> const preferences() const;

      private:
        friend class boost::serialization::access;
        template<class Archive>
          void serialize (Archive& ar, const unsigned int)
        {
          ar & BOOST_SERIALIZATION_NVP(_transition);
          ar & BOOST_SERIALIZATION_NVP(_transition_id);
          ar & BOOST_SERIALIZATION_NVP(_input);
          ar & BOOST_SERIALIZATION_NVP(_output);
        }

      private:
        we::type::transition_t _transition;
        boost::optional<we::transition_id_type> _transition_id;

        friend class net_type;
        input_t _input;
        output_t _output;
      };
    }
}
