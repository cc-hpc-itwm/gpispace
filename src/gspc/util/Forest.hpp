#pragma once

#include <util-generic/callable_signature.hpp>

#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  namespace util
  {
    // no cycle, antitransitive, no duplicate connections
    template<typename T>
      struct Forest
    {
      template<typename U> using From = U;
      template<typename U> using To = U;

      // insert a new connection
      // duplicate connection: throw, forest unchanged
      // insert would add a cycle: throw, forest unchanged
      // insert would add a diamond: throw, forest unchanged
      // amortized O(#component) for cycle/diamond detection
      Forest& insert (From<T>, To<T>);

      // insert a new connection without cycle detection
      // duplicate connection: throw, forest unchanged
      // amortized O(1)
      Forest& UNSAFE_insert (From<T>, To<T>);

      // remove an existing connection
      // nonexisting connection: throw, forest unchanged
      // amortized O(1)
      Forest& remove (From<T>, To<T>);

      // remove all incoming and all outgoing connections
      // amortized O(#neighbours)
      Forest& remove (T);

      // traverse downwards/upwards:
      // callback called once for the root and once for each transitive child
      // early abort: only go deeper if callback returns true
      // amortized O(#component)
      template<typename Arg, typename F>
        using is_callback = typename
          std::enable_if<fhg::util::is_callable<F, bool (Arg)>{}>::type;

      template<typename Callback, typename = is_callback<T, Callback>>
        void down (T, Callback) const;
      template<typename Callback, typename = is_callback<T, Callback>>
        void up (T, Callback) const;

      // iterate
      using Ts = std::unordered_set<T>;
      using Relation = std::unordered_map<T, Ts>;
      using Iterator = typename Relation::const_iterator;

      Iterator begin() const; // O(1)
      Iterator end() const;   // O(1)

      // transform
      template<typename U, typename F>
        using is_transformer = typename
          std::enable_if<fhg::util::is_callable<F, U (T const&)>{}>::type;

      // apply calls transform once for every node in the forest
      template< typename U
              , typename Transformer
              , typename = is_transformer<U, Transformer>
              >
        Forest<U> apply (Transformer) const;

      Forest() = default;
      Forest (Forest const&) = delete;
      Forest (Forest&&) = default;
      Forest& operator= (Forest const&) = delete;
      Forest& operator= (Forest&&) = delete;
      ~Forest() = default;

    private:
      Relation _suc;
      Relation _pre;
    };
  }
}

#include <gspc/util/Forest.ipp>
