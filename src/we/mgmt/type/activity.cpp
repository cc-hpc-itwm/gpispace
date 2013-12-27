// {petry,rahn}@itwm.fhg.de

#include <we/type/net.hpp>

#include <we/type/transition.hpp>

#include <we/expr/eval/context.hpp>

#include <we/mgmt/context.hpp>

#include <we/mgmt/type/activity.hpp>

#include <we/type/value/show.hpp>
#include <we/require_type.hpp>

#include <fhg/util/show.hpp>

#include <fhg/util/stat.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/foreach.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

namespace we
{
  namespace mgmt
  {
    namespace type
    {
      activity_t::activity_t ()
        : _id (petri_net::activity_id_invalid())
      {}

      activity_t::activity_t (const we::type::transition_t& transition)
        : _id (petri_net::activity_id_invalid())
        , _transition (transition)
      {}

      activity_t::activity_t (const activity_t& other)
        : _id (other._id)
        , _transition (other._transition)
        , _input(other._input)
        , _output (other._output)
      {}

      namespace
      {
        void decode (std::istream& s, we::mgmt::type::activity_t& t)
        {
          try
            {
              boost::archive::text_iarchive ar (s);

              ar >> BOOST_SERIALIZATION_NVP (t);
            }
          catch (boost::archive::archive_exception const &ex)
            {
              throw std::runtime_error
                (std::string ("deserialization error: ") + ex.what ());
            }
        }
      }

      activity_t::activity_t (const boost::filesystem::path& path)
      {
        std::ifstream stream (path.string().c_str());

        if (!stream)
          {
            throw std::runtime_error
              ("failed to open " + path.string() + "for reading");
          }

        decode (stream, *this);
      }

      activity_t::activity_t (std::istream& stream)
      {
        decode (stream, *this);
      }

      activity_t::activity_t (const std::string& s)
      {
        std::istringstream stream (s);

        decode (stream, *this);
      }

      activity_t& activity_t::operator= (const activity_t& other)
      {
        if (this != &other)
          {
            _id = other._id;
            _transition = (other._transition);
            _input = (other._input);
            _output = (other._output);
          }

        return *this;
      }

      std::string activity_t::to_string() const
      {
        std::ostringstream oss;
        boost::archive::text_oarchive ar (oss);
        ar << BOOST_SERIALIZATION_NVP (*this);
        return oss.str();
      }

      namespace
      {
        class visitor_activity_extractor
          : public boost::static_visitor<activity_t>
        {
        private:
          boost::mt19937& _engine;

        public:
          visitor_activity_extractor (boost::mt19937& engine)
            : _engine (engine)
          {}

          activity_t operator() (petri_net::net& net) const
          {
            return net.extract_activity_random (_engine);
          }

          template<typename T>
          activity_t operator() (T&) const
          {
            throw std::runtime_error ("STRANGE: activity_extractor");
          }
        };
      }

      activity_t activity_t::extract()
      {
        unique_lock_t lock (_mutex);

        return boost::apply_visitor ( visitor_activity_extractor (_engine)
                                    , _transition.data()
                                    );
      }

      namespace
      {
        class visitor_activity_injector : public boost::static_visitor<>
        {
        private:
          const activity_t& _child;

        public:
          visitor_activity_injector (const activity_t& child)
            : _child (child)
          {}

          void operator() (petri_net::net& parent) const
          {
            BOOST_FOREACH ( const activity_t::token_on_port_t& top
                          , _child.output()
                          )
              {
                parent.put_value
                  ( _child.transition().inner_to_outer().at (top.second).first
                  , top.first
                  );
              }
          }

          template <typename A>
          void operator() (A&) const
          {
            throw std::runtime_error ("STRANGE: activity_injector");
          }
        };
      }

      void activity_t::inject (const activity_t& subact)
      {
        unique_lock_t lock (_mutex);

        boost::apply_visitor ( visitor_activity_injector (subact)
                             , _transition.data()
                             );
      }

      namespace
      {
        class visitor_injector : public boost::static_visitor<void>
        {
        private:
          we::type::transition_t& _transition;
          const activity_t::input_t& _input;

        public:
          visitor_injector ( activity_t& activity
                           , const activity_t::input_t& input
                           )
            : _transition (activity.transition())
            , _input (input)
          {}

          void operator() (petri_net::net& net) const
          {
            BOOST_FOREACH (const activity_t::token_on_port_t& inp, _input)
              {
                const petri_net::port_id_type port_id (inp.second);

                if (_transition.get_port (port_id).has_associated_place())
                  {
                    net.put_value
                      ( _transition.get_port (port_id).associated_place()
                      , inp.first
                      );
                  }
              }
          }

          template<typename T>
          void operator() (T&) const
          {}
        };
      }

      void activity_t::inject_input()
      {
      }


      namespace
      {
        class visitor_add_input : public boost::static_visitor<>
        {
        private:
          we::type::transition_t& _transition;
          petri_net::port_id_type const& _port_id;
          pnet::type::value::value_type const& _value;

        public:
          visitor_add_input ( we::type::transition_t& transition
                            , petri_net::port_id_type const& port_id
                            , pnet::type::value::value_type const& value
                            )
            : _transition (transition)
            , _port_id (port_id)
            , _value (value)
          {}

          void operator() (petri_net::net& net) const
          {
            if (_transition.get_port (_port_id).has_associated_place())
            {
              net.put_value
                ( _transition.get_port (_port_id).associated_place()
                , _value
                );
            }
          }

          template<typename T>
          void operator() (T&) const
          {}
        };
      }

      void activity_t::add_input
        ( petri_net::port_id_type const& port_id
        , pnet::type::value::value_type const& value
        )
      {
        unique_lock_t const _ (_mutex);

        boost::apply_visitor ( visitor_add_input (_transition, port_id, value)
                             , _transition.data()
                             );

        _input.push_back (input_t::value_type (value, port_id));
      }

      namespace
      {
        class visitor_collect_output : public boost::static_visitor<>
        {
        private:
          activity_t& _activity;

        public:
          visitor_collect_output (activity_t& activity)
            : _activity (activity)
          {}

          void operator() (petri_net::net& net) const
          {
            BOOST_FOREACH
              ( we::type::transition_t::port_map_t::value_type const& p
              , _activity.transition().ports()
              )
            {
              if (p.second.is_output())
              {
                if (p.second.has_associated_place())
                {
                  const petri_net::port_id_type& port_id (p.first);
                  const petri_net::place_id_type& pid
                    (p.second.associated_place());

                  BOOST_FOREACH ( const pnet::type::value::value_type& token
                                , net.get_token (pid)
                                )
                  {
                    _activity.add_output
                      (activity_t::output_t::value_type (token, port_id));
                  }

                  net.delete_all_token (pid);
                }
                else
                {
                  throw std::runtime_error
                    ( "output port ("
                    + fhg::util::show (p.first)
                    + ", " + fhg::util::show (p.second) + ") "
                    + "is not associated with any place!"
                    );
                }
              }
            }
          }

          template<typename T>
          void operator() (T&) const
          {
            return;
          }
        };

      }

      void activity_t::collect_output ()
      {
        unique_lock_t lock (_mutex);

        boost::apply_visitor ( visitor_collect_output (*this)
                             , _transition.data()
                             );
      }

      const petri_net::activity_id_type& activity_t::id() const
      {
        return _id;
      }

      const we::type::transition_t& activity_t::transition() const
      {
        shared_lock_t lock (_mutex);
        return _transition;
      }

      we::type::transition_t& activity_t::transition()
      {
        unique_lock_t lock (_mutex);
        return _transition;
      }

      namespace
      {
        class executor : public boost::static_visitor<int>
        {
        private:
          type::activity_t& _activity;
          context* _ctxt;

        public:
          executor (type::activity_t& activity, context* ctxt)
            : _activity (activity)
            , _ctxt (ctxt)
          {}

          int operator () (petri_net::net& net) const
          {
            if (_activity.transition().is_internal())
              {
                return _ctxt->handle_internally (_activity, net);
              }
            else
              {
                return _ctxt->handle_externally (_activity, net);
              }
          }

          int operator() (we::type::module_call_t & mod) const
          {
            return _ctxt->handle_externally (_activity, mod);
          }

          int operator() (we::type::expression_t & expr) const
          {
            FHG_UTIL_STAT_INC ("expr " + expr.expression());
            FHG_UTIL_STAT_START ("expr " + expr.expression());

            expr::eval::context context;

            for ( type::activity_t::input_t::const_iterator top
                    (_activity.input().begin())
                ; top != _activity.input().end()
                ; ++top
                )
              {
                context.bind_ref
                  ( _activity.transition().get_port (top->second).name()
                  , top->first
                  );
              }

            FHG_UTIL_STAT_START ("expr-eval " + expr.expression());

            expr.ast ().eval_all (context);

            FHG_UTIL_STAT_STOP ("expr-eval " + expr.expression());
            FHG_UTIL_STAT_START ("expr-put " + expr.expression());

            BOOST_FOREACH
              ( we::type::transition_t::port_map_t::value_type const& p
              , _activity.transition().ports()
              )
            {
              if (p.second.is_output())
              {
                _activity.add_output
                  ( type::activity_t::output_t::value_type
                    ( pnet::require_type
                      ( context.value (p.second.name())
                      , p.second.signature()
                      , p.second.name()
                      )
                    , p.first
                    )
                  );
              }
            }

            FHG_UTIL_STAT_STOP ("expr-put " + expr.expression());
            FHG_UTIL_STAT_STOP ("expr " + expr.expression());

            return _ctxt->handle_internally (_activity, expr);
          }
        };
      }

      int activity_t::execute (context* ctxt)
      {
        unique_lock_t lock (_mutex);
        return boost::apply_visitor
          (executor (*this, ctxt), transition().data());
      }

      namespace
      {
        class visitor_can_fire : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const petri_net::net& net) const
          {
            return net.can_fire();
          }
          bool operator () (const we::type::module_call_t&) const
          {
            return false;
          }
          bool operator () (const we::type::expression_t&) const
          {
            return false;
          }
        };
      }

      bool activity_t::can_fire() const
      {
        shared_lock_t lock (_mutex);
        return boost::apply_visitor (visitor_can_fire(), transition().data());
      }

      const activity_t::input_t& activity_t::input() const
      {
        shared_lock_t lock (_mutex);
        return _input;
      }

      const activity_t::output_t& activity_t::output() const
      {
        shared_lock_t lock (_mutex);
        return _output;
      }

      void activity_t::set_output (const output_t& outp)
      {
        unique_lock_t lock (_mutex);
        _output = outp;
      }

      void activity_t::add_output (const output_t::value_type& outp)
      {
        unique_lock_t lock (_mutex);
        _output.push_back (outp);
      }

      void activity_t::lock()
      {
        _mutex.lock();
      }
      void activity_t::unlock()
      {
        _mutex.unlock();
      }

      std::ostream& activity_t::print ( std::ostream& os
                                      , const token_on_port_list_t& top_list
                                      ) const
      {
        bool first (true);

        os << "[";

        BOOST_FOREACH (const token_on_port_t& top, top_list)
          {
            if (first)
              {
                first = false;
              }
            else
              {
                os << ", ";
              }

            os << transition().get_port (top.second).name()
               << "=(" << pnet::type::value::show (top.first)
               << ", " << top.second << ")";
          }

        os << "]";

        return os;
      }

      namespace
      {
        class visitor_nice_name
          : public boost::static_visitor<boost::optional<std::string> >
        {
        public:
          boost::optional<std::string>
            operator() (const petri_net::net&) const
          {
            return boost::none;
          }
          boost::optional<std::string>
            operator() (const we::type::module_call_t& mod_call) const
          {
            return mod_call.module() + ":" + mod_call.function();
          }
          boost::optional<std::string>
            operator() (const we::type::expression_t&) const
          {
            return boost::none;
          }
        };
      }

      std::string activity_t::nice_name() const
      {
        shared_lock_t lock (_mutex);
        return boost::apply_visitor (visitor_nice_name(), transition().data())
          .get_value_or (transition().name());
      }

      bool operator== (const activity_t& a, const activity_t& b)
      {
        return a.id() == b.id();
      }
      std::ostream& operator<< (std::ostream& os, const activity_t& a)
      {
        return os << a.to_string();
      }
    }
  }
}
