// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_TODOT_HPP
#define WE_TYPE_BITS_TRANSITION_TODOT_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>

#include <we/type/value/show.hpp>
#include <we/type/signature/show.hpp>

#include <we/expr/parse/parser.hpp>
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/lexical_cast.hpp>

namespace we { namespace type {

    // ********************************************************************** //
    // toDot

    namespace dot {
      typedef unsigned long id_type;
      typedef unsigned long level_type;

      static const std::string endl = "\\n";
      static const std::string arrow = " -> ";

      // ******************************************************************* //

      inline
      std::ostream & level (std::ostream & s, const level_type & level)
      {
        for (level_type i (0); i < level; ++i)
          {
            s << "  ";
          }

        return s;
      }

      // ******************************************************************* //

      inline std::string parens ( const std::string & s
                                , const std::string open = "("
                                , const std::string close = ")"
                                )
      {
        return open + s + close;
      }

      inline std::string brackets (const std::string & s)
      {
        return " " + parens (s, "[", "]");
      }

      inline std::string props (const std::string & s)
      {
        return parens (s, ":: ", " ::");
      }

      inline std::string dquote (const std::string & s)
      {
        return parens (s, "\"", "\"");
      }

      // ******************************************************************* //

      inline std::string lines (const char & b, const std::string & s)
      {
        std::string l;

        std::string::const_iterator pos (s.begin());
        const std::string::const_iterator end (s.end());

        while (pos != end)
          {
            if (*pos == b)
              {
                ++pos;

                while (pos != end && (isspace (*pos) || *pos == b))
                  {
                    ++pos;
                  }

                l += endl;
              }
            else
              {
                l += *pos;

                ++pos;
              }
          }

        return l;
      }

      // ******************************************************************* //

      inline std::string quote (const char & c)
      {
        switch (c)
          {
          case '{': return "\\{";
          case '}': return "\\}";
          case '>': return "\\>";
          case '<': return "\\<";
          case '"': return "\\\"";
          case '|': return "\\|";
          default: return std::string (1, c);
          }
      }

      inline std::string quote (const std::string & s)
      {
        std::string q;

        for (std::string::const_iterator pos (s.begin()); pos != s.end(); ++pos)
          {
            q += quote (*pos);
          }

        return lines (';', q);
      }

      // ******************************************************************* //

      inline std::string keyval ( const std::string & key
                                , const std::string & val
                                )
      {
        return key + " = " + dquote (val);
      }

      inline std::string name (const id_type & id, const std::string & _name)
      {
        std::ostringstream s;

        s << "n" << id << "_" << _name;

        return s.str();
      }

      inline std::string bgcolor (const std::string & color)
      {
        std::ostringstream s;

        s << keyval ("bgcolor", color) << std::endl;

        return s.str();
      }

      // ******************************************************************* //

      namespace shape
      {
        static std::string condition;
        static std::string port_in;
        static std::string port_out;
        static std::string port_tunnel;
        static std::string expression;
        static std::string modcall;
        static std::string place;

        inline void init (const we::type::property::type & prop)
        {
          const std::string prefix ("pretty.dot.shape");

          condition = prop.get_with_default (prefix + ".condition", "record");
          port_in = prop.get_with_default (prefix + ".port-in", "house");
          port_out = prop.get_with_default (prefix + ".port-out", "invhouse");
          port_tunnel = prop.get_with_default (prefix + ".port-tunnel", "ellipse");
          expression = prop.get_with_default (prefix + ".expression", "none");
          modcall = prop.get_with_default (prefix + ".modcall", "box");
          place = prop.get_with_default (prefix + ".place", "ellipse");
        }

        template<typename Port>
        inline std::string port (const Port& p)
        {
          return p.is_input()
            ? port_in
            : (p.is_output() ? port_out : port_tunnel)
            ;
        }
      }

      namespace color
      {
        static std::string internal;
        static std::string external;
        static std::string modcall;
        static std::string expression;
        static std::string node;
        static std::string subnet_internal;

        inline void init (const we::type::property::type & prop)
        {
          const std::string prefix ("pretty.dot.color");

          internal = prop.get_with_default (prefix + ".internal", "white");
          external = prop.get_with_default (prefix + ".external", "dimgray");
          modcall = prop.get_with_default (prefix + ".modcall", "yellow");
          expression = prop.get_with_default (prefix + ".expression", "white");
          node = prop.get_with_default (prefix + ".node", "white");
          subnet_internal = prop.get_with_default ( prefix + ".subnet_internal"
                                                  , "grey"
                                                  );
        }
      }

      namespace style
      {
        static std::string association;
        static std::string read_connection;

        inline void init (const we::type::property::type & prop)
        {
          const std::string prefix ("pretty.dot.style");

          association =
            prop.get_with_default (prefix + ".association", "dotted");
          read_connection =
            prop.get_with_default (prefix + ".read-connection", "dashed");
        }
      }

      inline void init (const we::type::property::type & prop)
      {
        shape::init (prop);
        color::init (prop);
        style::init (prop);
      }

      // ******************************************************************* //

      inline std::string node ( const std::string & shape
                              , const std::string & label
                              )
      {
        std::ostringstream s;

        s << brackets ( keyval ("shape", shape)
                      + ", "
                      + keyval ("label", label)
                      + ", "
                      + keyval ("style", "filled")
                      + ", "
                      + keyval ("fillcolor", color::node)
                      )
          << std::endl
          ;

        return s.str();
      }

      template<typename Opts>
      inline std::string with_signature ( const std::string & name
                                        , const pnet::type::signature::signature_type & sig
                                        , const Opts & opts
                                        )
      {
        std::ostringstream s;

        s << name;

        if (opts.show_signature)
          {
            s << endl
              << pnet::type::signature::show (sig)
              ;
          }

        return s.str();
      }

      inline std::string association (void)
      {
        return brackets ( keyval ("style", style::association)
                        + ", "
                        + keyval ("dir", "none")
                        );
      }

      inline std::string property ( const std::string & prop
                                  , const std::string & val
                                  )
      {
        return props (quote (prop) + ": " + quote (val));
      }

      inline std::string property (const std::string & prop)
      {
        return props (quote (prop));
      }

      // ******************************************************************* //
      // predicates about when to expand a transition

      template<typename T> static bool all (const T &) { return true; };

      template<typename T>
      class generic
      {
      private:
        boost::function<bool (const T &)> f;

      public:
        generic () : f (all<T>) {}

        template<typename F>
        generic (F _f) : f (_f) {}

        bool operator () (const T & x) const
        {
          return f (x);
        }
      };

      template<typename Pred>
      class options
      {
      public:
        bool full;
        Pred predicate;
        bool show_token;
        bool show_signature;
        bool show_priority;
        bool show_intext;
        bool show_virtual;
        bool show_real;
        bool show_tunnel_connection;

        options ()
          : full (false)
          , predicate()
          , show_token (true)
          , show_signature (true)
          , show_priority (true)
          , show_intext (false)
          , show_virtual (false)
          , show_real (false)
          , show_tunnel_connection (true)
        {}
      };

      // ******************************************************************* //

      template <typename Pred>
      inline std::string to_dot
      ( const transition_t &
      , id_type &
      , const options<Pred> &
      , const level_type = 1
      , const petri_net::priority_type& = petri_net::priority_invalid()
      );

      template<typename Pred>
      class transition_visitor_dot : public boost::static_visitor<std::string>
      {
      private:
        id_type & id;
        const level_type l;
        const options<Pred> & opts;

      public:
        transition_visitor_dot ( id_type & _id
                               , const level_type & _l
                               , const options<Pred> & _opts
                               )
          : id (_id)
          , l (_l)
          , opts (_opts)
        {}

        // ----------------------------------------------------------------- //

        std::string operator () (const expression_t & expr) const
        {
          std::ostringstream s;

          level (s, l)
            << name (id, "expression")
            << node (shape::expression, quote (boost::lexical_cast<std::string> (expr)))
            ;

          return s.str();
        }

        // ----------------------------------------------------------------- //

        std::string operator () (const module_call_t & mod_call) const
        {
          std::ostringstream s;

          level (s, l)
            << name (id, "modcall")
            << node (shape::modcall, boost::lexical_cast<std::string> (mod_call))
            ;

          return s.str();
        }

        // ----------------------------------------------------------------- //

        std::string operator ()
        (const petri_net::net & net) const
        {
          typedef transition_t::port_map_t::value_type pmv_t;
          typedef std::pair< std::string
                           , petri_net::port_id_type
                           > extra_connection_type;
          typedef boost::unordered_map< std::string
                                      , std::list<extra_connection_type>
                                      > extra_connection_by_transition_type;

          std::ostringstream s;
          extra_connection_by_transition_type ecbt;

          const id_type id_net (id);

          level (s, l)
            << "subgraph cluster_net_" << id_net << " {"
            << std::endl;

          typedef std::pair<petri_net::place_id_type, place::type> ip_type;

          BOOST_FOREACH (const ip_type& ip, net.places())
            {
              const petri_net::place_id_type& place_id (ip.first);
              const place::type& place (ip.second);
              const std::string place_dot_name
                (name (id_net, "place_" + boost::lexical_cast<std::string> (place_id)));

              std::ostringstream token;

              if (opts.show_token)
                {
                  BOOST_FOREACH ( const pnet::type::value::value_type& t
                                , net.get_token (place_id)
                                )
                  {
                    token << endl << pnet::type::value::show (t);
                  }
                }

              std::ostringstream virt;

              if (opts.show_virtual)
                {
                  if (  "true"
                     == place.property().get_with_default ( "virtual"
                                                          , "false"
                                                          )
                     )
                    {
                      virt << endl << property ("virtual");
                    }
                }

              level (s, l + 1)
                << place_dot_name
                << node
                   ( shape::place
                   , with_signature ( place.name()
                                    , place.signature()
                                    , opts
                                    )
                   + quote (token.str())
                   + virt.str()
                   )
                ;
            }

          typedef std::pair<petri_net::transition_id_type,transition_t> it_type;

          BOOST_FOREACH (const it_type& it, net.transitions())
            {
              const petri_net::transition_id_type& trans_id (it.first);
              const transition_t& trans (it.second);
              const id_type id_trans (++id);
              const petri_net::priority_type prio
                (net.get_transition_priority (trans_id));

              s << to_dot (trans, id, opts, l + 1, prio);

              if (opts.show_tunnel_connection)
                {
                  BOOST_FOREACH ( const extra_connection_type& ec
                                , ecbt[trans.name()]
                                )
                    {
                      level (s, l + 1)
                        << name (id_trans, "port_" + boost::lexical_cast<std::string>(ec.second))
                        << arrow
                        << ec.first
                        << association()
                        << std::endl
                        ;
                    }
                }

              if (  net.port_to_place().find (trans_id)
                 != net.port_to_place().end()
                 )
              {
                BOOST_FOREACH
                  ( petri_net::net::port_to_place_with_info_type::value_type
                  const& port_to_place
                  , net.port_to_place().at (trans_id)
                  )
                {
                  level (s, l + 1)
                    << name ( id_trans
                            , "port_" + boost::lexical_cast<std::string> (port_to_place.get_left())
                            )
                    << arrow
                    << name ( id_net
                            , "place_" + boost::lexical_cast<std::string> (port_to_place.get_right())
                            )
                    << std::endl
                    ;
                }
              }

              if (net.place_to_port().find (trans_id) !=  net.place_to_port().end())
              {
                BOOST_FOREACH
                  ( petri_net::net::place_to_port_with_info_type::value_type
                  const& place_to_port
                  , net.place_to_port().at (trans_id)
                  )
                {
                  level (s, l + 1)
                    << name ( id_net
                            , "place_" + boost::lexical_cast<std::string> (place_to_port.get_left())
                            )
                    << arrow
                    << name ( id_trans
                            , "port_" + boost::lexical_cast<std::string> (place_to_port.get_right())
                            )
                    << (  net.place_to_transition_read().find
                       ( petri_net::net::adj_pt_type::value_type
                       (place_to_port.get_left(), trans_id)
                       )
                       != net.place_to_transition_read().end()
                       ? brackets (keyval ("style", style::read_connection))
                       : ""
                       )
                    << std::endl
                    ;
                }
              }
            }

          level (s, l + 1) << bgcolor (color::internal);

          level (s, l)
            << "} /* " << "cluster_net_" << id_net << " */"
            << std::endl;

          return s.str();
        }
      };

      // ******************************************************************* //

      template <typename Pred>
      inline std::string to_dot
      ( const transition_t & t
      , id_type & id
      , const options<Pred> & opts
      , const level_type l
      , const petri_net::priority_type & prio
      )
      {
        std::ostringstream s;

        const id_type id_trans (id);

        level (s, l)
          << "subgraph cluster_" << id_trans << " {"
          << std::endl;

        std::ostringstream priority;

        if (opts.show_priority)
          {
            if (prio != petri_net::priority_invalid())
              {
                if (prio > petri_net::priority_type (0))
                  {
                    priority << "| priority: " << prio;
                  }
              }
          }

        std::ostringstream intext;

        if (opts.show_intext)
          {
            intext << "|" << (t.is_internal() ? "internal" : "external");
          }

        std::ostringstream cond;

        if (boost::lexical_cast<std::string> (t.condition()) != "true")
        {
          std::ostringstream oss;
          oss << expr::parse::parser (boost::lexical_cast<std::string> (t.condition()));

          cond << "|" << lines ('&', quote (oss.str()));
        }

        level (s, l + 1)
          << name (id_trans, "condition")
          << node ( shape::condition
                  , t.name()
                  + cond.str()
                  + intext.str()
                  + priority.str()
                  )
          ;

        BOOST_FOREACH ( typename transition_t::port_map_t::value_type const& p
                      , t.ports()
                      )
        {
          level (s, l + 1)
            << name (id_trans, "port_" + boost::lexical_cast<std::string> (p.first))
            << node ( shape::port (p.second)
                    , with_signature ( p.second.name()
                                     , p.second.signature()
                                     , opts
                                     )
                    )
            ;
        }

        if (opts.predicate (t))
          {
            s << boost::apply_visitor
                 (transition_visitor_dot<Pred> (id, l + 1, opts), t.data());

            BOOST_FOREACH ( typename transition_t::port_map_t::value_type const& p
                          , t.ports()
                          )
            {
              if (p.second.has_associated_place())
              {
                level (s, l + 1)
                  << name (id_trans, "port_" + boost::lexical_cast<std::string> (p.first))
                  << arrow
                  << name (id_trans
                          , "place_"
                          + boost::lexical_cast<std::string> (p.second.associated_place())
                          )
                  << association()
                  << std::endl
                  ;
              }
            }
          }

        level (s, l + 1)
          << bgcolor ( t.expression() ? color::expression
                     : t.module_call() ? color::modcall
                     : t.is_internal() ? color::subnet_internal
                     : color::external
                     );

        level (s, l)
          << "} /* " << "cluster_" << id_trans << " == " << t.name() << " */"
          << std::endl;

        return s.str();
      }
    }
  }
}

#endif
