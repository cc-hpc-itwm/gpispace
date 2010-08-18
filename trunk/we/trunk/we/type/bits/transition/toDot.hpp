// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_TODOT_HPP
#define WE_TYPE_BITS_TRANSITION_TODOT_HPP 1

#include <we/type/transition.hpp>

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
        bool was_b (false);

        for (std::string::const_iterator pos (s.begin()); pos != s.end(); ++pos)
          {
            if (*pos == b)
              {
                l += endl;

                was_b = true;
              }
            else
              {
                if (was_b)
                  {
                    if (!isspace (*pos))
                      {
                        was_b = false;

                        l += *pos;
                      }
                  }
                else
                  {
                    l += *pos;
                  }
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
          default: return fhg::util::show (c);
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
        static std::string expression;
        static std::string modcall;
        static std::string place;

        inline void init (const we::type::property::type & prop)
        {
          const std::string prefix ("pretty.dot.shape");

          condition = prop.get_with_default (prefix + ".condition", "record");
          port_in = prop.get_with_default (prefix + ".port-in", "house");
          port_out = prop.get_with_default (prefix + ".port-out", "invhouse");
          expression = prop.get_with_default (prefix + ".expression", "none");
          modcall = prop.get_with_default (prefix + ".modcall", "box");
          place = prop.get_with_default (prefix + ".place", "ellipse");
        }
      }

      namespace color
      {
        static std::string internal;
        static std::string external;
        static std::string modcall;
        static std::string expression;
        static std::string node;

        inline void init (const we::type::property::type & prop)
        {
          const std::string prefix ("pretty.dot.color");

          internal = prop.get_with_default (prefix + ".internal", "white");
          external = prop.get_with_default (prefix + ".external", "grey");
          modcall = prop.get_with_default (prefix + ".modcall", "yellow");
          expression = prop.get_with_default (prefix + ".expression", "white");
          node = prop.get_with_default (prefix + ".node", "white");
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
                                        , const signature::type & sig
                                        , const Opts & opts
                                        )
      {
        std::ostringstream s;

        s << name;

        if (opts.show_signature)
          {
            s << endl
              << lines (',', opts.full ? fhg::util::show (sig.desc()) : sig.nice())
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
        bool show_capacity;
        bool show_signature;
        bool show_priority;
        bool show_intext;
        bool show_virtual;
        bool show_real;

        options ()
          : full (false)
          , predicate()
          , show_token (true)
          , show_capacity (true)
          , show_signature (true)
          , show_priority (true)
          , show_intext (false)
          , show_virtual (true)
          , show_real (true)
        {}
      };

      // ******************************************************************* //

      template <typename P, typename E, typename T, typename Pred>
      inline std::string dot
      ( const transition_t<P,E,T> &
      , id_type &
      , const options<Pred> &
      , const level_type = 1
      , const petri_net::prio_t &
      = petri_net::traits::id_traits<petri_net::prio_t>::invalid()
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
            << node (shape::expression, quote (fhg::util::show (expr)))
            ;

          return s.str();
        }

        // ----------------------------------------------------------------- //

        std::string operator () (const module_call_t & mod_call) const
        {
          std::ostringstream s;

          level (s, l)
            << name (id, "modcall")
            << node (shape::modcall, fhg::util::show (mod_call))
            ;

          return s.str();
        }

        // ----------------------------------------------------------------- //

        template <typename P, typename E, typename T>
        std::string operator ()
        (const petri_net::net<P, transition_t<P, E, T>, E, T> & net) const
        {
          typedef transition_t<P, E, T> transition_t;
          typedef petri_net::net<P, transition_t, E, T> pnet_t;
          typedef typename pnet_t::place_const_it place_const_it;
          typedef petri_net::adj_place_const_it adj_place_const_it;
          typedef typename pnet_t::transition_const_it transition_const_it;
          typedef typename pnet_t::token_place_it token_place_it;
          typedef petri_net::connection_t connection_t;

          std::ostringstream s;

          const id_type id_net (id);

          level (s, l)
            << "subgraph cluster_net_" << id_net << " {"
            << std::endl;

          for (place_const_it p (net.places()); p.has_more(); ++p)
            {
              const P place (net.get_place (*p));

              std::ostringstream token;

              if (opts.show_token)
                {
                  typedef boost::unordered_map<T, size_t> token_cnt_t;
                  token_cnt_t token_cnt;
                  for ( token_place_it tp (net.get_token (*p))
                      ; tp.has_more()
                      ; ++tp
                      )
                    {
                      ++token_cnt[*tp];
                    }

                  for ( typename token_cnt_t::const_iterator
                          tok (token_cnt.begin())
                      ; tok != token_cnt.end()
                      ; ++tok
                      )
                    {
                      token << endl;

                      if (tok->second > 1)
                        {
                          token << tok->second << " x ";
                        }

                      token << quote (fhg::util::show (tok->first));
                    }
                }

              std::ostringstream capacity;

              if (opts.show_capacity)
                {
                  try
                    {
                      capacity << endl << "capacity: " << net.get_capacity (*p);
                    }
                  catch (const petri_net::exception::capacity_unbounded &)
                    {
                      // do nothing, there is no capacity given
                    }
                }

              std::ostringstream virt;

              if (opts.show_virtual)
                {
                  if (  "true"
                     == place.get_property().get_with_default ( "virtual"
                                                              , "false"
                                                              )
                     )
                    {
                      virt << endl << property ("virtual");
                    }
                }

              std::ostringstream real;

              if (opts.show_real)
                {
                  namespace prop = we::type::property::traverse;

                  prop::stack_type stack
                    (prop::dfs (place.get_property(), "real"));

                  while (!stack.empty())
                    {
                      real << endl
                           << property ("real"
                                       , stack.top().first[0]
                                       + "."
                                       + stack.top().second
                                       )
                        ;

                      stack.pop();
                    }
                }

              level (s, l + 1)
                << name (id_net, "place_" + fhg::util::show (*p))
                << node
                   ( shape::place
                   , with_signature ( place.get_name()
                                    , place.get_signature()
                                    , opts
                                    )
                   + token.str()
                   + capacity.str()
                   + virt.str()
                   + real.str()
                   )
                ;
            }

          for (transition_const_it t (net.transitions()); t.has_more(); ++t)
            {
              const transition_t & trans (net.get_transition (*t));
              const id_type id_trans (++id);
              const petri_net::prio_t prio (net.get_transition_priority (*t));

              s << dot<P, E, T> (trans, id, opts, l + 1, prio);

              for ( typename transition_t::inner_to_outer_t::const_iterator
                      connection (trans.inner_to_outer_begin())
                  ; connection != trans.inner_to_outer_end()
                  ; ++connection
                  )
                {
                  level (s, l + 1)
                    << name ( id_trans
                            , "port_" + fhg::util::show (connection->first)
                            )
                    << arrow
                    << name ( id_net
                            , "place_" + fhg::util::show (connection->second.first)
                            )
                    << std::endl
                    ;
                }

              for ( typename transition_t::outer_to_inner_t::const_iterator
                      connection (trans.outer_to_inner_begin())
                  ; connection != trans.outer_to_inner_end()
                  ; ++connection
                  )
                {
                  bool found (false);
                  bool is_read (false);

                  for ( adj_place_const_it p (net.in_to_transition (*t))
                      ; p.has_more()
                      ; ++p
                      )
                    {
                      if (*p == connection->first)
                        {
                          found = true;

                          const connection_t net_conn (net.get_edge_info (p()));

                          if (petri_net::is_pt_read (net_conn.type))
                            {
                              is_read = true;
                            }

                          break;
                        }
                    }

                  if (!found)
                    {
                      throw std::runtime_error
                         ("STRANGE! Connected in port but not in net!");
                    }

                  level (s, l + 1)
                    << name ( id_net
                            , "place_" + fhg::util::show (connection->first)
                            )
                    << arrow
                    << name ( id_trans
                            , "port_" + fhg::util::show (connection->second.first)
                            )
                    << ( is_read
                       ? brackets (keyval ("style", style::read_connection))
                       : ""
                       )
                    << std::endl
                    ;
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

      template <typename P, typename E, typename T, typename Pred>
      inline std::string dot
      ( const transition_t<P,E,T> & t
      , id_type & id
      , const options<Pred> & opts
      , const level_type l = 1
      , const petri_net::prio_t & prio
      = petri_net::traits::id_traits<petri_net::prio_t>::invalid()
      )
      {
        typedef transition_t<P,E,T> trans_t;

        std::ostringstream s;

        const id_type id_trans (id);

        level (s, l)
          << "subgraph cluster_" << id_trans << " {"
          << std::endl;

        std::ostringstream priority;

        if (opts.show_priority)
          {
            if (prio != petri_net::traits::id_traits<petri_net::prio_t>::invalid())
              {
                if (prio > 0)
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

        if (fhg::util::show (t.condition()) != "true")
          {
            cond << "|" << lines ('&', quote (fhg::util::show (t.condition())));
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

        for ( typename trans_t::const_iterator p (t.ports_begin())
            ; p != t.ports_end()
            ; ++p
            )
          {
            level (s, l + 1)
              << name (id_trans, "port_" + fhg::util::show(p->first))
              << node ( p->second.is_input() ? shape::port_in : shape::port_out
                      , with_signature ( p->second.name()
                                       , p->second.signature()
                                       , opts
                                       )
                      )
              ;
          }

        if (opts.predicate (t))
          {
            s << boost::apply_visitor
                 (transition_visitor_dot<Pred> (id, l + 1, opts), t.data());

            for ( typename trans_t::const_iterator p (t.ports_begin())
                ; p != t.ports_end()
                ; ++p
                )
              {
                if (p->second.has_associated_place())
                  {
                    level (s, l + 1)
                      << name (id_trans, "port_" + fhg::util::show (p->first))
                      << arrow
                      << name (id_trans
                              , "place_"
                              + fhg::util::show (p->second.associated_place())
                              )
                      << association()
                      << std::endl
                      ;
                  }
              }
          }

        switch (boost::apply_visitor (content::visitor (), t.data()))
          {
          case content::modcall:
            level (s, l + 1) << bgcolor (color::modcall);
            break;
          case content::expression:
            level (s, l + 1) << bgcolor (color::expression);
            break;
          case content::subnet:
            level (s, l + 1) <<
              bgcolor (t.is_internal() ? color::internal : color::external)
              ;
            break;
          default: throw std::runtime_error
              ("STRANGE: unknown type of transition content");
          }

        level (s, l)
          << "} /* " << "cluster_" << id_trans << " == " << t.name() << " */"
          << std::endl;

        return s.str();
      }
    }
  }
}

#endif
