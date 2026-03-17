// Copyright (C) 2013-2014,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/peek.hpp>

#include <gspc/we/type/value/path/split.hpp>

#include <iterator>



    namespace gspc::pnet::type::value
    {
      namespace
      {
        template<typename V, typename M, typename MIT>
        class visitor_peek : public ::boost::static_visitor<std::optional<std::reference_wrapper<V>>>
        {
        public:
          visitor_peek ( std::list<std::string>::const_iterator const& key
                       , std::list<std::string>::const_iterator const& end
                       , V& node
                       )
            : _key (key)
            , _end (end)
            , _node (node)
          {}

          std::optional<std::reference_wrapper<V>> operator() (M& m) const
          {
            if (_key == _end)
            {
              return _node;
            }

            MIT field (m.begin());

            while (field != m.end())
            {
              if (field->first == *_key)
              {
                return ::boost::apply_visitor
                  ( visitor_peek<V,M,MIT> ( std::next (_key)
                                          , _end
                                          , field->second
                                          )
                  , field->second
                  );
              }

              ++field;
            }

            return {};
          }

          template<typename T>
          std::optional<std::reference_wrapper<V>> operator() (T&) const
          {
            if (_key == _end)
            {
              return _node;
            }

            return {};
          }

        private:
          std::list<std::string>::const_iterator const& _key;
          std::list<std::string>::const_iterator const& _end;
          V& _node;
        };
      }

      std::optional<std::reference_wrapper<value_type const>>
      peek ( std::list<std::string>::const_iterator const& key
           , std::list<std::string>::const_iterator const& end
           , value_type const& node
           )
      {
        return ::boost::apply_visitor
          ( visitor_peek< const value_type
                        , const structured_type
                        , structured_type::const_iterator
                        > (key, end, node)
          , node
          );
      }
      std::optional<std::reference_wrapper<value_type const>>
      peek (std::list<std::string> const& path, value_type const& node)
      {
        return peek (path.begin(), path.end(), node);
      }
      std::optional<std::reference_wrapper<value_type const>>
      peek (std::string const& path, value_type const& node)
      {
        return peek (path::split (path), node);
      }

      std::optional<std::reference_wrapper<value_type>>
      peek ( std::list<std::string>::const_iterator const& key
           , std::list<std::string>::const_iterator const& end
           , value_type& node
           )
      {
        return ::boost::apply_visitor
          ( visitor_peek< value_type
                        , structured_type
                        , structured_type::iterator
                        > (key, end, node)
          , node
          );
      }
      std::optional<std::reference_wrapper<value_type>>
      peek (std::list<std::string> const& path, value_type& node)
      {
        return peek (path.begin(), path.end(), node);
      }
      std::optional<std::reference_wrapper<value_type>>
      peek (std::string const& path, value_type& node)
      {
        return peek (path::split (path), node);
      }
    }
