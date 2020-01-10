#include <boost/format.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/serialization/boost/blank.hpp>

#include <algorithm>
#include <exception>
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
      void erase_all ( T x
                     , Relation& forward
                     , Relation& backward
                     , Annotations& annotations
                     )
    {
      if (!annotations.erase (x))
      {
        throw std::invalid_argument ("Unknown.");
      }

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

      return;
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
  }

  template<typename T, typename A>
    bool Forest<T, A>::is_unknown (T const& x) const
  {
    return _annotations.end() == _annotations.find (x);
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
    void Forest<T, A>::remove_leaf (T x)
  try
  {
    if (_suc.find (x) != _suc.end())
    {
      throw std::invalid_argument ("Not a leaf.");
    }

    detail::erase_all<T> (x, _pre, _suc, _annotations);
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
        (str (boost::format ("Forest::remove_leaf: ('%1%')") % x))
      );
  }

  template<typename T, typename A>
    void Forest<T, A>::remove_root (T x)
  try
  {
    if (_pre.find (x) != _pre.end())
    {
      throw std::invalid_argument ("Not a root.");
    }

    detail::erase_all<T> (x, _suc, _pre, _annotations);
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
    template< typename CombiningTransformer
            , typename U
            , typename B
            , typename
            >
    Forest<U, B>
    Forest<T, A>::upward_combine_transform (CombiningTransformer) const
  {
    throw std::runtime_error ("NYI: upward_combine_transform");
  }

  template<typename T, typename A>
    template< typename UnorderedTransformer
            , typename U
            , typename B
            , typename
            >
    Forest<U, B>
    Forest<T, A>::unordered_transform (UnorderedTransformer) const
  {
    throw std::runtime_error ("NYI: unordered_transform");
  }

  template<typename T, typename A>
    template<typename Archive>
    void Forest<T, A>::serialize (Archive& ar, unsigned int /* version */)
  {
    ar & _suc;
    ar & _pre;
    ar & _annotations;
  }
}
