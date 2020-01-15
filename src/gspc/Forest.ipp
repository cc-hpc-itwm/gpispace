#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/serialization/boost/blank.hpp>

#include <algorithm>
#include <exception>
#include <iterator>
#include <stack>

namespace gspc
{
  namespace detail
  {
    template<typename Key, typename Relation, typename Callback>
      void for_each_at ( Relation const& relation
                       , Key const& key
                       , Callback&& callback
                       )
    {
      auto const pos (relation.find (key));

      if (pos != relation.cend())
      {
        std::for_each ( pos->second.cbegin()
                      , pos->second.cend()
                      , std::forward<Callback> (callback)
                      );
      }
    }

    template<typename T, typename Container>
      typename Container::iterator
        assert_element (T x, Container& xs)
    {
      auto pos {xs.find (x)};

      if (pos == xs.end())
      {
        throw std::logic_error ("INCONSISTENCY.");
      }

      return pos;
    }

    template<typename Outer, typename Middle, typename Inner>
      void erase ( Outer& outer
                 , Middle& middle
                 , Inner& inner
                 )
    {
      middle->second.erase (inner);

      if (middle->second.empty())
      {
        outer.erase (middle);
      }

      return;
    }

    template<typename T, typename Relation, typename Annotations>
      typename Annotations::value_type
        erase_all ( T x
                  , Relation& forward
                  , Relation& backward
                  , Annotations& annotations
                  )
    {
      auto const node_it (annotations.find (x));

      if (node_it == annotations.end())
      {
        throw std::invalid_argument ("Unknown.");
      }

      auto node (*node_it);

      annotations.erase (node_it);

      auto pos {forward.find (x)};

      if (pos != forward.end())
      {
        for (auto child : pos->second)
        {
          auto sop {assert_element<T> (child, backward)};
          auto sop_x {assert_element<T> (x, sop->second)};

          erase (backward, sop, sop_x);
        }

        forward.erase (pos);
      }

      return node;
    }

    template< typename T
            , typename Callback
            , typename Relation
            , typename Annotations
            >
      void component
      ( T root
      , Callback callback
      , Relation const& relation
      , Annotations const& annotations
      )
    {
      auto open {std::stack<T>{}};

      open.push (root);

      while (!open.empty())
      {
        auto index {open.top()};
        open.pop();

        callback (*annotations.find (index));

        for_each_at (relation, index, [&] (auto child) { open.push (child); });
      }

      return;
    }

    template<typename Key, typename Container>
      bool is_member (Key const& key, Container const& relation)
    {
      return relation.find (key) != relation.end();
    }
  }

  template<typename T, typename A>
    bool Forest<T, A>::is_unknown (T const& x) const
  {
    return !detail::is_member (x, _annotations);
  }
  template<typename T, typename A>
    bool Forest<T, A>::is_leaf (T const& x) const
  {
    return !detail::is_member (x, _suc);
  }
  template<typename T, typename A>
    bool Forest<T, A>::is_root (T const& x) const
  {
    return !detail::is_member (x, _pre);
  }

  template<typename T, typename A>
    T const& Forest<T, A>::assert_is_known (T const& x) const
  {
    if (is_unknown (x))
    {
      throw std::invalid_argument ("Unknown.");
    }

    return x;
  }
  template<typename T, typename A>
    T const& Forest<T, A>::assert_is_root (T const& x) const
  {
    if (!is_root (x))
    {
      throw std::invalid_argument ("Not a root.");
    }

    return x;
  }
  template<typename T, typename A>
    T const& Forest<T, A>::assert_is_leaf (T const& x) const
  {
    if (!is_leaf (x))
    {
      throw std::invalid_argument ("Not a leaf.");
    }

    return x;
  }

  template<typename T, typename A>
    template<typename Callback, typename>
      void Forest<T, A>::for_each_node (Callback&& callback) const
  {
    std::for_each (_annotations.cbegin(), _annotations.cend(), callback);
  }

  template<typename T, typename A>
    template<typename Callback, typename>
      void Forest<T, A>::for_each_root (Callback&& callback) const
  {
    for_each_node ( [&] (forest::Node<T, A> const& node)
                    {
                      if (is_root (node.first))
                      {
                        callback (node);
                      }
                    }
                  );
  }

  template<typename T, typename A>
    template<typename Callback, typename>
      void Forest<T, A>::for_each_leaf (Callback&& callback) const
  {
    for_each_node ( [&] (forest::Node<T, A> const& node)
                    {
                      if (is_leaf (node.first))
                      {
                        callback (node);
                      }
                    }
                  );
  }

  template<typename T, typename A>
    forest::Node<T, A> const&
      Forest<T, A>::insert (forest::Node<T, A> node, Children tos)
  {
    return insert ( std::move (node.first)
                  , std::move (node.second)
                  , std::move (tos)
                  );
  }

  template<typename T, typename A>
    forest::Node<T, A> const&
      Forest<T, A>::insert (T from, A annotation, Children tos)
  try
  {
    auto is_known
      ( [&] (auto const& x)
        {
          return !is_unknown (x);
        }
      );

    if (is_known (from))
    {
      throw std::invalid_argument ("Duplicate.");
    }

    if (!std::all_of (tos.cbegin(), tos.cend(), is_known))
    {
      throw std::invalid_argument ("Unknown child");
    }

    {
      Children seen;

      std::for_each ( tos.cbegin()
                    , tos.cend()
                    , [&] (auto child)
                      {
                        down ( std::move (child)
                             , [&] (auto const& node)
                               {
                                 if (!seen.emplace (node.first).second)
                                 {
                                   throw std::invalid_argument ("Diamond.");
                                 }
                               }
                            );
                      }
                    );
    }

    for (auto const& to : tos)
    {
      if (!_suc[from].emplace (to).second || !_pre[to].emplace (from).second)
      {
        throw std::logic_error ("INCONSISTENCY.");
      }
    }

    return *_annotations.emplace (from, annotation).first;
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
        ( ( boost::format ("Forest::insert: (from '%1%', to %2%)")
          % from
          % fhg::util::print_container ("{'", "', '", "'}", tos)
          ).str()
        )
      );
  }

  template<typename T, typename A>
    auto Forest<T, A>::remove_leaf (T x) -> Node
  try
  {
    return detail::erase_all<T> (assert_is_leaf (x), _pre, _suc, _annotations);
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
        (str (boost::format ("Forest::remove_leaf: ('%1%')") % x))
      );
  }

  template<typename T, typename A>
    auto Forest<T, A>::remove_root (T x) -> Node
  try
  {
    return detail::erase_all<T> (assert_is_root (x), _suc, _pre, _annotations);
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
        (str (boost::format ("Forest::remove_root: ('%1%')") % x))
      );
  }

  template<typename T, typename A>
    template<typename Callback, typename>
    void Forest<T, A>::down (T root, Callback callback) const
  {
    return detail::component<T>
      (assert_is_known (root), std::move (callback), _suc, _annotations);
  }
  template<typename T, typename A>
    template<typename Callback, typename>
    void Forest<T, A>::up (T root, Callback callback) const
  {
    return detail::component<T>
      (assert_is_known (root), std::move (callback), _pre, _annotations);
  }

  template<typename T, typename A>
    template<typename Callback, typename>
      void Forest<T, A>::down_up (T root, Callback&& callback) const
  {
    return down ( std::move (root)
                , [&] (Node const& child) { up (child.first, callback); }
                );
  }

  template<typename T, typename A>
    template< typename CombiningTransformer
            , typename U
            , typename B
            , typename
            >
    Forest<U, B>
    Forest<T, A>::upward_combine_transform (CombiningTransformer function) const
  {
    Forest<U, B> transformed_forest;

    //! BEGIN STATE
    std::unordered_map<T, forest::Node<U, B> const*> transformed_nodes;
    std::unordered_map<T, std::size_t> missing_children_by_key;
    std::stack<T> nodes;

    auto is_last_unseen_child
      ( [&] (T const& parent, T const& /* child: ignored, only counted */)
        {
          auto missing_children (missing_children_by_key.find (parent));

          if (missing_children == missing_children_by_key.end())
          {
            throw std::logic_error
              ("INCONSISTENCY: Parent without missing child.");
          }

          --missing_children->second;

          if (missing_children->second == 0)
          {
            missing_children_by_key.erase (missing_children);

            return true;
          }

          return false;
        }
      );

    auto transformed_children_and_results
      ( [&] (T const& node)
        {
          typename Forest<U, B>::Children children;
          std::list<forest::Node<U, B> const*> results;

          detail::for_each_at
            ( _suc
            , node
            , [&] (auto const& child)
              {
                auto const& transformed_node (transformed_nodes.at (child));

                children.emplace (transformed_node->first);
                results.emplace_back (transformed_node);
              }
            );

          return std::make_tuple (std::move (children), std::move (results));
        }
      );
    //! END STATE

    for_each_leaf
      ( [&] (forest::Node<T, A> const& node)
        {
          nodes.push (node.first);
        }
      );

    for (auto const& suc : _suc)
    {
      missing_children_by_key[suc.first] = suc.second.size();
    }

    while (!nodes.empty())
    {
      auto const t (nodes.top());
      nodes.pop();

      auto tcrs (transformed_children_and_results (t));
      auto& children (std::get<0> (tcrs));
      auto& results (std::get<1> (tcrs));

      transformed_nodes.emplace
        ( t
        , &transformed_forest.insert
          ( function (*_annotations.find (t), std::move (results))
          , std::move (children)
          )
        );

      detail::for_each_at
        ( _pre
        , t
        , [&] (T const& parent)
          {
            if (is_last_unseen_child (parent, t))
            {
              nodes.push (parent);
            }
          }
        );
    }

    //! BEGIN SANITY
    if (!missing_children_by_key.empty())
    {
      throw std::logic_error ("INCONSISTENCY: Missing childs!");
    }
    if (transformed_nodes.size() != _annotations.size())
    {
      throw std::logic_error ("INCONSISTENCY: Not transformed all nodes!");
    }
    //! END SANITY

    return transformed_forest;
  }

  template<typename T, typename A>
    template< typename UnorderedTransformer
            , typename U
            , typename B
            , typename
            >
    Forest<U, B>
    Forest<T, A>::unordered_transform (UnorderedTransformer function) const
  {
    return upward_combine_transform
      ( [&] ( forest::Node<T, A> const& node
            , std::list<forest::Node<U, B> const*> const& // children_results
            )
        {
          return function (node);
        }
      );
  }

  template<typename T, typename A>
    template<typename Pred, typename>
      Forest<T, A> Forest<T, A>::remove_root_if (Pred&& pred) &&
  {
    std::stack<T> roots;

    for_each_root
      ( [&] (forest::Node<T, A> const& node)
        {
          roots.push (node.first);
        }
      );

    while (!roots.empty())
    {
      auto const root (roots.top());
      roots.pop();

      if (pred (*_annotations.find (root)))
      {
        detail::for_each_at
          ( _suc
          , root
          , [&] (T const& child)
            {
              if (_pre.at (child).size() == 1) // <==> pre.at (child) == {root}
              {
                roots.push (child);
              }
            }
          );

        remove_root (root);
      }
    }

    return std::move (*this);
  }


  template<typename T, typename A>
    void Forest<T, A>::merge (Forest<T, A> other)
  try
  {
    for (auto const& key : other._annotations | boost::adaptors::map_keys)
    {
      if (_annotations.count (key))
      {
        throw std::invalid_argument ("Duplicate Key.");
      }
    }

    return UNSAFE_merge (std::move (other));
  }
  catch (...)
  {
    std::throw_with_nested (std::runtime_error ("Forest::merge: (other)"));
  }

  template<typename T, typename A>
    void Forest<T, A>::UNSAFE_merge (Forest<T, A> other)
  {
    _suc.insert ( std::make_move_iterator (other._suc.begin())
                , std::make_move_iterator (other._suc.end())
                );
    _pre.insert ( std::make_move_iterator (other._pre.begin())
                , std::make_move_iterator (other._pre.end())
                );
    _annotations.insert ( std::make_move_iterator (other._annotations.begin())
                        , std::make_move_iterator (other._annotations.end())
                        );
  }

  template<typename T, typename A>
    template<typename SplitKey, typename Key, typename>
      std::unordered_map<Key, Forest<T, A>>
        Forest<T, A>::multiway_split (SplitKey&& split_key) const
  {
    auto node_keys
      ( [&] (auto const& children)
        {
          std::unordered_set<T> ts;
          std::transform ( children.cbegin(), children.cend()
                         , std::inserter (ts, ts.end())
                         , [] (auto const* child) { return child->first; }
                         );
          return ts;
        }
      );

    std::unordered_map<T, Key> split_keys_by_node_key;

    auto assert_all_children_have_the_same_split_key
      ( [&] (auto const& key, auto const& children)
        {
          if ( !std::all_of
                  ( children.cbegin(), children.cend()
                  , [&] (auto const* child)
                    {
                      return key == split_keys_by_node_key.at (child->first);
                    }
                  )
             )
          {
            throw std::invalid_argument
              ("Forest::multiway_split: "
              " Connected component crosses key boundary."
              );
          }
        }
      );

    std::unordered_map<Key, Forest<T, A>> result;

    upward_combine_transform
      ( [&] (Node const& node, std::list<Node const*> const& children)
        {
          auto const key
            ( split_keys_by_node_key.emplace (node.first, split_key (node))
            . first->second
            );

          assert_all_children_have_the_same_split_key (key, children);

          result[key].insert (node, node_keys (children));

          return node;
        }
      );

    return result;
  }

  template<typename T, typename A>
    template<typename Callback, typename>
    void Forest<T, A>::upward_apply (Callback&& function) const
  {
    upward_combine_transform
      ( [&] ( Node const& node
            , std::list<Node const*> const& // unused children
            )
        {
          function (node);

          return node;
        }
      );
  }

  template<typename T, typename A>
    template<typename Archive>
    void Forest<T, A>::serialize (Archive& ar, unsigned int /* version */)
  {
    ar & _suc;
    ar & _pre;
    ar & _annotations;
  }

  template<typename A, typename T>
    UniqueForest<A, T>::UniqueForest (Forest<T, A> forest)
      : Forest<T, A> (std::move (forest))
      , _next_key
        ( [&]
          {
            return this->_annotations.empty()
              ? 0
              : 1 + std::max_element ( this->_annotations.begin()
                                     , this->_annotations.end()
                                     , [&] (auto const& lhs, auto const& rhs)
                                       {
                                         return lhs.first < rhs.first;
                                       }
                                     )->first
              ;
          }()
        )
  {}

  template<typename A, typename T>
    T UniqueForest<A, T>::insert
      ( A value
      , typename Forest<T, A>::Children const& children
      )
  {
    return Forest<T, A>::insert
      (_next_key++, std::move (value), std::move (children)).first;
  }

  template<typename A, typename T>
    template<typename Archive>
      void UniqueForest<A, T>::serialize
        (Archive& ar, unsigned int /* version */)
  {
    ar & static_cast<Forest<T, A>&> (*this);
    ar & _next_key;
  }
}
