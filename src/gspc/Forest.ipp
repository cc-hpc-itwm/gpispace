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

    template<typename T, typename Relation>
      void erase_all ( T x
                     , Relation& forward
                     , Relation& backward
                     )
    {
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

        auto children {relation.find (index)};

        if (children != relation.end())
        {
          for (auto child : children->second)
          {
            open.push (child);
          }
        }
      }

      return;
    }
  }

  template<typename T, typename A>
    void Forest<T, A>::insert (T from, A annotation, Children tos)
    try
    {
      auto is_unknown
        ( [&] (auto x)
          {
            return 0 == _annotations.count (x);
          }
          );

      if (!is_unknown (from))
      {
        throw std::invalid_argument ("Duplicate.");
      }

      if (std::any_of (tos.cbegin(), tos.cend(), is_unknown))
      {
        throw std::invalid_argument ("Unknown child");
      }

      {
        Ts seen;

        std::for_each ( tos.cbegin()
                      , tos.cend()
                      , [&] (T x)
                        {
                          if (!seen.emplace (std::move (x)).second)
                          {
                            throw std::invalid_argument ("Diamond.");
                          }
                        }
                      );
      }

      _annotations.emplace (from, annotation);

      for (auto const& to : tos)
      {
        if (!_suc[from].emplace (to).second || !_pre[to].emplace (from).second)
        {
          throw std::logic_error ("INCONSISTENCY.");
        }
      }
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

      if (!_annotations.erase (x))
      {
        throw std::invalid_argument ("Unknown.");
      }

      detail::erase_all<T> (x, _suc, _pre);
      detail::erase_all<T> (x, _pre, _suc);
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

      if (!_annotations.erase (x))
      {
        throw std::invalid_argument ("Unknown.");
      }

      detail::erase_all<T> (x, _suc, _pre);
      detail::erase_all<T> (x, _pre, _suc);
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
    return detail::component<T> (root, callback, _suc, _annotations);
  }
  template<typename T, typename A>
    template<typename Callback, typename>
    void Forest<T, A>::up (T root, Callback callback) const
  {
    return detail::component<T> (root, callback, _pre, _annotations);
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
