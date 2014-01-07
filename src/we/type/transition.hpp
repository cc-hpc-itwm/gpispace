// {petry, rahn}@itwm.fhg.de

#ifndef WE_TYPE_TRANSITION_HPP
#define WE_TYPE_TRANSITION_HPP 1

#include <we/type/port.hpp>
#include <we/type/module_call.hpp>
#include <we/type/expression.hpp>
#include <we/type/condition.hpp>
#include <we/type/signature.hpp>
#include <we/type/property.hpp>
#include <we/type/id.hpp>
#include <we/type/requirement.hpp>
#include <we/type/value.hpp>

#include <we/type/net.fwd.hpp>

#include <we/expr/eval/context.hpp>

#include <we/exception.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/version.hpp>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/map.hpp>

#include <boost/lexical_cast.hpp>

#include <stdexcept>

namespace we { namespace type {
    struct transition_t
    {
    private:
      typedef boost::variant< module_call_t
                            , expression_t
                            , boost::recursive_wrapper<petri_net::net>
                            > data_type;

      typedef std::pair< petri_net::place_id_type
                       , we::type::property::type
                       > pid_with_prop_t;

    public:
      typedef std::pair< petri_net::port_id_type
                       , we::type::property::type
                       > port_id_with_prop_t;
      typedef boost::unordered_map< petri_net::place_id_type
                                  , port_id_with_prop_t
                                  > outer_to_inner_t;
      typedef boost::unordered_map< petri_net::port_id_type
                                  , pid_with_prop_t
                                  > inner_to_outer_t;

      typedef boost::unordered_map<petri_net::port_id_type, port_t> port_map_t;

      transition_t ()
        : name_ ("<<transition unknown>>")
        , data_ (expression_t())
        , internal_ (true)
        , condition_("true")
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
      {}

      template <typename Type>
      transition_t ( const std::string& name
                   , Type const& typ
                   , condition::type const& _condition
                   , bool intern
                   , const we::type::property::type& prop
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (intern)
        , condition_ (_condition)
        , outer_to_inner_()
        , inner_to_outer_()
        , ports_()
        , port_id_counter_ (0)
        , prop_(prop)
        , _requirements()
      { }

      boost::optional<const expression_t&> expression() const;
      boost::optional<const petri_net::net&> net() const;
      boost::optional<const module_call_t&> module_call() const;

      const condition::type& condition() const;
      const std::string& name() const;
      bool is_internal() const;
      void set_internal (bool);
      const data_type& data() const;
      data_type& data();
      std::list<we::type::requirement_t> const& requirements() const;

    private:
      void connect_outer_to_inner ( const petri_net::place_id_type&
                                  , const petri_net::port_id_type&
                                  , const we::type::property::type&
                                  );
      void connect_inner_to_outer ( const petri_net::port_id_type&
                                  , const petri_net::place_id_type&
                                  , const we::type::property::type&
                                  );

    public:
      void re_connect_inner_to_outer ( const petri_net::port_id_type&
                                     , const petri_net::place_id_type&
                                     , const we::type::property::type&
                                     );
      void re_connect_outer_to_inner ( const petri_net::place_id_type&
                                     , const petri_net::place_id_type&
                                     , const petri_net::port_id_type&
                                     , const we::type::property::type&
                                     );

      inner_to_outer_t const& inner_to_outer() const;
      outer_to_inner_t const& outer_to_inner() const;

      void add_connection ( petri_net::place_id_type const&
                          , petri_net::port_id_type const&
                          , we::type::property::type const&
                          );

      void remove_connection_in (const petri_net::place_id_type&);
      void remove_connection_out (const petri_net::port_id_type&);

      void add_connection ( petri_net::port_id_type const&
                          , petri_net::place_id_type const&
                          , we::type::property::type const&
                          );

      petri_net::port_id_type add_port (port_t const&);
      void erase_port (const petri_net::port_id_type&);

      petri_net::port_id_type input_port_by_name (const std::string&) const;
      const petri_net::port_id_type& output_port_by_name (const std::string&) const;

      const port_t& get_port (const petri_net::port_id_type& port_id) const;
      port_t& get_port (const petri_net::port_id_type& port_id);

      // UNSAFE: does not check for multiple connections! Use with care!
      //! \todo remove
      void UNSAFE_re_associate_port ( const petri_net::place_id_type&
                                    , const petri_net::place_id_type&
                                    );

      const we::type::property::type& prop() const;

      const port_map_t& ports() const;
      port_map_t& ports();

      void add_requirement (we::type::requirement_t const&);

      template<typename T>
        boost::optional<T> get_schedule_data
          ( std::vector<std::pair< pnet::type::value::value_type
                                 , petri_net::port_id_type
                                 >
                       > const& input
          , const std::string& key
          ) const
      {
        we::type::property::path_type path;
        path.push_back ("fhg");
        path.push_back ("drts");
        path.push_back ("schedule");
        path.push_back (key);

        boost::optional<const property::value_type&> expr
          (prop().get_maybe_val (path));

        if (!expr)
        {
          return boost::none;
        }

        expression_t e (*expr);

        expr::eval::context context;

        typedef std::pair< pnet::type::value::value_type
                         , petri_net::port_id_type
                         > token_on_port_t;

        BOOST_FOREACH (token_on_port_t const& top, input)
        {
          context.bind_ref (get_port (top.second).name(), top.first);
        }

        return boost::get<T> (e.ast().eval_all (context));
      }


    private:
      std::string name_;
      data_type data_;
      bool internal_;
      condition::type condition_;

      outer_to_inner_t outer_to_inner_;
      inner_to_outer_t inner_to_outer_;
      port_map_t ports_;
      petri_net::port_id_type port_id_counter_;

      we::type::property::type prop_;

      std::list<we::type::requirement_t> _requirements;

      friend class boost::serialization::access;
      template <typename Archive>
      void save(Archive& ar, const unsigned int) const
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(data_);
        ar & BOOST_SERIALIZATION_NVP(internal_);
        ar & BOOST_SERIALIZATION_NVP(condition_);
        ar & BOOST_SERIALIZATION_NVP(outer_to_inner_);
        ar & BOOST_SERIALIZATION_NVP(inner_to_outer_);
        ar & BOOST_SERIALIZATION_NVP(ports_);
        ar & BOOST_SERIALIZATION_NVP(port_id_counter_);
        ar & BOOST_SERIALIZATION_NVP(prop_);
        ar & BOOST_SERIALIZATION_NVP(_requirements);
      }

      template <typename Archive>
      void load(Archive & ar, const unsigned int version)
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(data_);
        ar & BOOST_SERIALIZATION_NVP(internal_);
        ar & BOOST_SERIALIZATION_NVP(condition_);
        ar & BOOST_SERIALIZATION_NVP(outer_to_inner_);
        ar & BOOST_SERIALIZATION_NVP(inner_to_outer_);
        ar & BOOST_SERIALIZATION_NVP(ports_);
        ar & BOOST_SERIALIZATION_NVP(port_id_counter_);
        ar & BOOST_SERIALIZATION_NVP(prop_);

        if (version > 0)
        {
          ar & BOOST_SERIALIZATION_NVP(_requirements);
        }
      }
      BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    // ********************************************************************* //

    boost::optional<transition_t::port_id_with_prop_t>
    output_port_by_pid (transition_t const&, const petri_net::place_id_type&);

    boost::optional<transition_t::port_id_with_prop_t const&>
    input_port_by_pid (transition_t const&, const petri_net::place_id_type&);

    boost::unordered_set<std::string>
    port_names (transition_t const&, const we::type::PortDirection&);

    boost::optional<const port_t&>
    get_port_by_associated_pid ( transition_t const&
                               , const petri_net::place_id_type&
                               );
  }
}

BOOST_CLASS_VERSION(we::type::transition_t, 1)

#endif
