// {petry, rahn}@itwm.fhg.de

#ifndef WE_TYPE_TRANSITION_HPP
#define WE_TYPE_TRANSITION_HPP 1

#include <we/expr/eval/context.hpp>
#include <we/type/expression.hpp>
#include <we/type/id.hpp>
#include <we/type/module_call.hpp>
#include <we/type/net.fwd.hpp>
#include <we/type/port.hpp>
#include <we/type/property.hpp>
#include <we/type/requirement.hpp>
#include <we/type/value.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

namespace we { namespace type {
    struct transition_t
    {
    private:
      typedef boost::variant< module_call_t
                            , expression_t
                            , boost::recursive_wrapper<we::type::net_type>
                            > data_type;

      typedef std::pair< we::place_id_type
                       , we::type::property::type
                       > pid_with_prop_t;

    public:
      typedef std::pair< we::port_id_type
                       , we::type::property::type
                       > port_id_with_prop_t;
      typedef boost::unordered_map<we::port_id_type, port_t> port_map_t;

      transition_t ()
        : name_ ("<<transition unknown>>")
        , data_ (expression_t())
        , internal_ (true)
        , condition_ (boost::none)
        , ports_()
        , port_id_counter_ (0)
        , prop_()
        , _requirements()
        , _priority()
      {}

      template <typename Type>
      transition_t ( const std::string& name
                   , Type const& typ
                   , boost::optional<expression_t> const& _condition
                   , bool intern
                   , const we::type::property::type& prop
                   , we::priority_type priority
                   )
        : name_ (name)
        , data_ (typ)
        , internal_ (intern)
          //! \todo check is_const_true, better check user input earlier
        , condition_ (_condition)
        , ports_()
        , port_id_counter_ (0)
        , prop_(prop)
        , _requirements()
        , _priority (priority)
      { }

      const std::string& name() const;

      const data_type& data() const;
      data_type& data();

      boost::optional<const expression_t&> expression() const;
      boost::optional<const we::type::net_type&> net() const;
      boost::optional<const module_call_t&> module_call() const;

      bool is_internal() const;

      boost::optional<expression_t> const& condition() const;

      we::port_id_type add_port (port_t const&);

      we::port_id_type input_port_by_name (const std::string&) const;
      const we::port_id_type& output_port_by_name (const std::string&) const;

      const port_map_t& ports() const;

      const we::type::property::type& prop() const;

      std::list<we::type::requirement_t> const& requirements() const;
      void add_requirement (we::type::requirement_t const&);

      we::priority_type priority() const;

      template<typename T>
        boost::optional<T> get_schedule_data
          ( std::vector<std::pair< pnet::type::value::value_type
                                 , we::port_id_type
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
          (prop().get (path));

        if (!expr)
        {
          return boost::none;
        }

        expression_t e (*expr);

        expr::eval::context context;

        typedef std::pair< pnet::type::value::value_type
                         , we::port_id_type
                         > token_on_port_t;

        BOOST_FOREACH (token_on_port_t const& top, input)
        {
          context.bind_ref (ports().at (top.second).name(), top.first);
        }

        return boost::get<T> (e.ast().eval_all (context));
      }

    private:
      std::string name_;
      data_type data_;
      bool internal_;
      boost::optional<expression_t> condition_;

      port_map_t ports_;
      we::port_id_type port_id_counter_;

      we::type::property::type prop_;

      std::list<we::type::requirement_t> _requirements;
      we::priority_type _priority;

      friend class boost::serialization::access;
      template <typename Archive>
      void serialize(Archive & ar, const unsigned int version)
      {
        ar & BOOST_SERIALIZATION_NVP(name_);
        ar & BOOST_SERIALIZATION_NVP(data_);
        ar & BOOST_SERIALIZATION_NVP(internal_);
        ar & BOOST_SERIALIZATION_NVP(condition_);
        ar & BOOST_SERIALIZATION_NVP(ports_);
        ar & BOOST_SERIALIZATION_NVP(port_id_counter_);
        ar & BOOST_SERIALIZATION_NVP(prop_);
        ar & BOOST_SERIALIZATION_NVP(_requirements);
        ar & BOOST_SERIALIZATION_NVP(_priority);
      }
    };
  }
}

#endif
