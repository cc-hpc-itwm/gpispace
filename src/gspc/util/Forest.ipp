#include <boost/format.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>

#include <exception>
#include <stack>
#include <stdexcept>

namespace gspc
{
  namespace util
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

      template<typename T, typename Callback, typename Relation>
        void component
          ( T root
          , Callback callback
          , Relation const& relation
          )
      {
        auto open {std::stack<T>{}};

        open.push (root);

        while (!open.empty())
        {
          auto index {open.top()};
          open.pop();

          if (callback (index))
          {
            auto children {relation.find (index)};

            if (children != relation.end())
            {
              for (auto child : children->second)
              {
                open.push (child);
              }
            }
          }
        }

        return;
      }
    }

    template<typename T>
      Forest<T>& Forest<T>::insert
        (From<T> from, To<T> to)
    try
    {
      down ( to
           , [&] (T index)
             {
               if (index == from)
               {
                 throw std::invalid_argument ("Cycle.");
               }

               return true;
             }
           );

      {
        auto seen {Ts{}};
        auto see
          ( [&] (T index)
            {
              if (!seen.emplace (std::move (index)).second)
              {
                throw std::invalid_argument ("Diamond.");
              }

              return true;
            }
          );

        up (to, see);
        up (from, see);
      }

      return UNSAFE_insert (from, to);
    }
    catch (...)
    {
      std::throw_with_nested
        ( std::runtime_error
          ( ( boost::format ("Forest::insert: (from %1%, to %2%)")
            % from
            % to
            ).str()
          )
        );
    }

    template<typename T>
      Forest<T>& Forest<T>::UNSAFE_insert
        (From<T> from, To<T> to)
    try
    {
      if (!_suc[from].emplace (to).second)
      {
        throw std::invalid_argument ("Duplicate connection.");
      }

      if (!_pre[to].emplace (from).second)
      {
        throw std::logic_error ("INCONSISTENCY.");
      }

      return *this;
    }
    catch (...)
    {
      std::throw_with_nested
        ( std::runtime_error
          ( ( boost::format ("Forest::UNSAFE_insert: (from %1%, to %2%)")
            % from
            % to
            ).str()
          )
          );
    }

    template<typename T>
      Forest<T>& Forest<T>::remove
        (From<T> from, To<T> to)
    try
    {
      auto suc {_suc.find (from)};

      if (suc == _suc.end())
      {
        throw std::invalid_argument
          ("Connection does not exist: Missing from.");
      }

      auto suc_to {suc->second.find (to)};

      if (suc_to == suc->second.end())
      {
        throw std::invalid_argument
          ("Connection does not exist: Missing to.");
      }

      auto pre {detail::assert_element<T> (to, _pre)};
      auto pre_from {detail::assert_element<T> (from, pre->second)};

      detail::erase (_suc, suc, suc_to);
      detail::erase (_pre, pre, pre_from);

      return *this;
    }
    catch (...)
    {
      std::throw_with_nested
        ( std::runtime_error
          ( ( boost::format ("Forest::remove: (from %1%, to %2%)")
            % from
            % to
            ).str()
          )
        );
    }

    template<typename T>
      Forest<T>& Forest<T>::remove (T x)
    try
    {
      detail::erase_all<T> (x, _suc, _pre);
      detail::erase_all<T> (x, _pre, _suc);

      return *this;
    }
    catch (...)
    {
      std::throw_with_nested
        ( std::runtime_error
          ( ( boost::format ("Forest::remove: (id %1%)")
            % x
            ).str()
          )
        );
    }

    template<typename T>
      template<typename Callback, typename>
        void Forest<T>::down (T root, Callback callback) const
    {
      return detail::component<T> (root, callback, _suc);
    }
    template<typename T>
      template<typename Callback, typename>
        void Forest<T>::up (T root, Callback callback) const
    {
      return detail::component<T> (root, callback, _pre);
    }

    template<typename T>
      template<typename CombiningTransformer, typename>
     AnnotatedForest< T
                    , MaybeError<fhg::util::return_type<CombiningTransformer>>
                    >
      Forest<T>::combining_transform (CombiningTransformer) const
    {
      throw std::runtime_error ("NYI: combining_transform");
    }

    template<typename T>
      template<typename Archive>
        void Forest<T>::serialize (Archive& ar, unsigned int /* version */)
    {
      ar & _suc;
      ar & _pre;
    }
  }
}
