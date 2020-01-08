#pragma once

//! \todo BEGIN
#include <boost/serialization/list.hpp>
#include <boost/serialization/utility.hpp>
#include <list>
#include <utility>

namespace gspc
{
  namespace util
  {
    //! \note NOT forest<pair<t...>> but forest needs to maintain id -> t.
    //! *AnnotatedForest::iterator == std::pair<ID, T>
    template<typename ID, typename T> using AnnotatedForest
      = std::list<std::pair<ID, T>>;
  }
}
//! \todo END

#include <gspc/MaybeError.hpp>

#include <util-generic/callable_signature.hpp>

#include <boost/serialization/access.hpp>

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
      Forest() = default;
      Forest (Forest const&) = default;       //! \todo rpc requires copy
      Forest (Forest&&) = default;
      Forest& operator= (Forest const&) = delete;
      Forest& operator= (Forest&&) = delete;
      ~Forest() = default;

      template<typename U> using From = U;
      template<typename U> using To = U;

      // insert a new connection
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

      //! \todo: iterate
      using Ts = std::unordered_set<T>;
      using Relation = std::unordered_map<T, Ts>;

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

      template<typename F, typename U = fhg::util::return_type<F>>
        using is_combining_transformer = std::enable_if_t
          < fhg::util::is_callable
              < F
              , U ( T const& // node
                  , std::list<MaybeError<U> const*> const& // results of all direct children
                  )
              >{}
          >;
      //! applies \a transformer to parent only if all children did not throw
      //! returned tree has the same shape as the parameter
      template< typename CombiningTransformer
              , typename = is_combining_transformer<CombiningTransformer>
              >
     AnnotatedForest< T
                    , MaybeError<fhg::util::return_type<CombiningTransformer>>
                    >
        combining_transform (CombiningTransformer) const
      {
        throw std::runtime_error ("NYI: combining_transform");
      }

    private:
      Relation _suc;
      Relation _pre;

      friend boost::serialization::access;
      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };
  }
}

#include <gspc/util/Forest.ipp>
