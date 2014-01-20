// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_HPP 1

#include <pnete/ui/graph/style/type.fwd.hpp>
#include <pnete/ui/graph/style/fallback.hpp>
#include <pnete/ui/graph/style/store.hpp>
#include <pnete/ui/graph/mode.hpp>

#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>

#include <QGraphicsItem>

class QPainter;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class base_item;

        namespace style
        {
          namespace detail
          {
            template<typename T, typename Item>
              T fallback (const Item* item, const key_type& key)
            {
              if ( const Item* parent
                 = qgraphicsitem_cast<const Item*> (item->parentItem())
                 )
                {
                  return parent->template style<T> (key);
                }

              return fallback::get<T> (key, item->mode());
            }
          }

          namespace store
          {
            template<typename T>
            class of
            {
            public:
              typedef cached_predicates<const base_item*, T> type;
            };

            typedef boost::variant < of<qreal>::type
                                   , of<QBrush>::type
                                   , of<QColor>::type
                                   , of<Qt::PenStyle>::type
                                   > type;

            namespace visitor
            {
              template<typename T>
              class mk : public boost::static_visitor<type&>
              {
              private:
                type& _x;

              public:
                mk (type& x) : _x (x) {}

                type& operator () (T&) const { return _x; }

                template<typename U>
                type& operator () (const U&) const { return _x = T(); }
              };
            }

            template<typename T> type& mk_or_keep (type& x)
            {
              return boost::apply_visitor (visitor::mk<T> (x), x);
            }

            void clear_cache (const type& x);
          }

          namespace
          {
            template<typename VALUE>
              boost::optional<VALUE> constant
              (const VALUE& val, const base_item*)
            {
              return val;
            }
          }

          class type
          {
          private:
            typedef boost::unordered_map<mode::type, store::type> by_mode_type;
            typedef boost::unordered_map< key_type
                                        , by_mode_type
                                        > by_mode_by_key_type;

            by_mode_by_key_type _by_mode_by_key;

          public:
            void clear_cache();

            //! \todo implement other accessors
            template<typename T>
            void
            push ( const key_type& key
                 , const mode::type& mode
                 , const typename store::of<T>::type::predicate_type& pred
                 )
            {
              typedef typename store::of<T>::type store_type;

              by_mode_by_key_type::iterator by_mode
                (_by_mode_by_key.find (key));

              if (by_mode == _by_mode_by_key.end())
                {
                  by_mode_type bm;
                  bm.insert ( by_mode_type::value_type ( mode
                                                       , store_type (pred)
                                                       )
                            );

                  _by_mode_by_key.insert
                    (by_mode_by_key_type::value_type (key, bm));
                }
              else
                {
                  by_mode_type::iterator store (by_mode->second.find (mode));

                  if (store == by_mode->second.end())
                    {
                      by_mode->second.insert
                        ( by_mode_type::value_type ( mode
                                                   , store_type (pred)
                                                   )
                        );
                    }
                  else
                    {
                      boost::get<store_type&> ( store::mk_or_keep<store_type>
                                                (store->second)
                                              )
                        .push (pred)
                        ;
                    }
                }
            }

            template<typename VALUE>
              void push ( const key_type& key
                        , const mode::type& mode
                        , const VALUE& value
                        )
            {
              push<VALUE> (key, mode, boost::bind (constant<VALUE>, value, _1));
            }

            template<typename T>
            void
            push ( const key_type& key
                 , const typename store::of<T>::type::predicate_type& pred
                 )
            {
              push<T> (key, mode::NORMAL, pred);
              push<T> (key, mode::HIGHLIGHT, pred);
              push<T> (key, mode::DRAG, pred);
              push<T> (key, mode::MOVE, pred);
              push<T> (key, mode::CONNECT, pred);
              push<T> (key, mode::CONNECT_ALLOWED, pred);
              push<T> (key, mode::CONNECT_FORBIDDEN, pred);
            }

            template<typename VALUE>
              void push ( const key_type& key
                        , const VALUE& value
                        )
            {
              push<VALUE> (key, mode::NORMAL, value);
              push<VALUE> (key, mode::HIGHLIGHT, value);
              push<VALUE> (key, mode::DRAG, value);
              push<VALUE> (key, mode::MOVE, value);
              push<VALUE> (key, mode::CONNECT, value);
              push<VALUE> (key, mode::CONNECT_ALLOWED, value);
              push<VALUE> (key, mode::CONNECT_FORBIDDEN, value);
            }


            template<typename T>
              T get ( const base_item* item
                    , const mode::type& mode
                    , const key_type& key
                    ) const
            {
              typedef typename store::of<T>::type store_type;

              const by_mode_by_key_type::const_iterator by_mode
                (_by_mode_by_key.find (key));

              if (by_mode == _by_mode_by_key.end())
                {
                  return detail::fallback<T, base_item> (item, key);
                }

              const by_mode_type::const_iterator store
                (by_mode->second.find (mode));

              if (store == by_mode->second.end())
                {
                  return detail::fallback<T, base_item> (item, key);
                }

              typename store::of<T>::type::optional_value_type value
                (boost::get<const store_type&> (store->second).get (item));

              if (value)
                {
                  return *value;
                }

              return detail::fallback<T, base_item> (item, key);
            }
          };

          void draw_shape (const base_item*, QPainter* painter);
        }
      }
    }
  }
}

#endif
