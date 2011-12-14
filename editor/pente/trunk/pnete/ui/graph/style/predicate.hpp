// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_PREDICATE_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_PREDICATE_HPP 1

#include <string>

#include <boost/function.hpp>
#include <boost/optional.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class item;

        namespace style
        {
          namespace predicate
          {
            namespace detail
            {
              template<typename Derived>
              class base
              {
              public:
                bool operator () (const graph::item* gi) const
                {
                  return static_cast<const Derived*>(this)->operator ()(gi);
                }
              };

              template<typename L, typename R>
              class _and : public base< _and<L,R> >
              {
              private:
                const L _l;
                const R _r;

              public:
                explicit _and (const L& l, const R& r) : _l (l), _r (r) {}

                bool operator () (const graph::item* gi) const
                {
                  return _l (gi) && _r (gi);
                }
              };

              template<typename L, typename R>
              class _or : public base< _or<L,R> >
              {
              private:
                const L _l;
                const R _r;

              public:
                explicit _or (const L& l, const R& r) : _l (l), _r (r) {}

                bool operator () (const graph::item* gi) const
                {
                  return _l (gi) || _r (gi);
                }
              };

              template<typename L>
              class _not : public base< _not<L> >
              {
              private:
                const L _l;

              public:
                explicit _not (const L& l) : _l (l) {}

                bool operator () (const graph::item* gi) const
                {
                  return !_l (gi);
                }
              };
            }

            typedef boost::function<bool (const graph::item*)> function_type;

            class predicate : public detail::base<predicate>
            {
            private:
              const function_type _function;

            public:
              explicit predicate (const function_type&);
              bool operator () (const graph::item*) const;
            };

            template<typename T>
            class on : public detail::base< on<T> >
            {
            private:
              typedef boost::function<const T& (const graph::item*)> select_type;
              typedef boost::function<bool (const T&)> apply_type;

              const select_type _select;
              const apply_type _apply;

            public:
              explicit on ( const select_type& select
                          , const apply_type& apply
                          )
                : _select (select)
                , _apply (apply)
              {}
              bool operator () (const graph::item* gi) const
              {
                return _apply (_select (gi));
              }
            };

            template<typename L, typename R>
            detail::_and<L,R> _and (const L& l, const R& r)
            {
              return detail::_and<L,R> (l, r);
            }
            template<typename L, typename R>
            detail::_or<L,R> _or (const L& l, const R& r)
            {
              return detail::_or<L,R> (l, r);
            }
            template<typename L>
            detail::_not<L> _not (const L& l)
            {
              return detail::_not<L> (l);
            }

            template<typename T, typename P>
            boost::optional<const T&>
            generic_if ( const P& pred
                       , const T& x
                       , const graph::item* item
                       )
            {
              if (pred (item))
                {
                  return x;
                }

              return boost::none;
            }

            bool is_connection (const graph::item*);
            bool is_port (const graph::item*);
            bool is_transition (const graph::item*);
            bool is_place (const graph::item*);
            bool is_top_level_port (const graph::item*);

            bool starts_with (const std::string&, const std::string&);
            bool ends_with (const std::string&, const std::string&);
            bool equals (const std::string&, const std::string&);

            namespace port
            {
              const std::string& name (const graph::item *);
              const std::string& type (const graph::item *);
            }

            namespace transition
            {
              const std::string& name (const graph::item *);
//               const bool internal (const graph::item *);
//               const bool inline (const graph::item *);
//               const bool is_expression (const graph::item *);
//               const bool is_module_call (const graph::item *);
//               const bool is_subnet (const graph::item *);
            }

            namespace place
            {
//               const std::string& name (const graph::item *);
//               const std::string& type (const graph::item *);
//               const bool is_virtual (const graph::item *);
            }

            namespace connection
            {
            }
          }
        }
      }
    }
  }
}

#endif
