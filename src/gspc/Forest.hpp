#pragma once

#include <util-generic/callable_signature.hpp>

#include <boost/serialization/access.hpp>
#include <boost/blank.hpp>

#include <list>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace gspc
{
  namespace forest
  {
    //! \note implies std::pair<T const, A> and probably more copies
    //! than necessary
    template<typename T, typename A = boost::blank>
      using Node = typename std::unordered_map<T, A>::value_type;
  }

  // no cycle, no diamond
  template<typename T, typename A = boost::blank>
    struct Forest
  {
    Forest() = default;
    Forest (Forest const&) = default;       //! \todo rpc requires copy
    Forest (Forest&&) = default;
    Forest& operator= (Forest const&) = delete;
    Forest& operator= (Forest&&) = delete;
    ~Forest() = default;

    // 1 throw when duplicate x
    // 2 throw when any_of (cs: unknown)
    // 3 throw when cycle -> is free because of 1, 2
    // 4 throw when diamond -> seen = cs; for c : cs: traverse and throw if seen twice
    using Children = std::unordered_set<T>;

    void insert (T x, A a, Children cs);

    void remove_leaf (T);
    void remove_root (T);

    // traverse downwards/upwards:
    // callback called once for the root and once for each transitive child
    // early abort: only go deeper if callback returns true
    // amortized O(#component)
    template<typename F>
      using is_callback =
        fhg::util::is_callable<F, void (forest::Node<T, A> const&)>;

    template< typename Callback
            , typename = std::enable_if_t<is_callback<Callback>{}>
            >
      void down (T, Callback) const;

    template< typename Callback
            , typename = std::enable_if_t<is_callback<Callback>{}>
            >
      void up (T, Callback) const;

    //! combining transformer
    template<typename F, typename U, typename B>
      using is_combining_transformer = std::enable_if_t
      < fhg::util::is_callable
        < F
        , forest::Node<U, B>
          ( forest::Node<T, A> const&
          , std::list<forest::Node<U, B> const*> const& // results of all direct children
          )
        >{}
      >;

    template< typename CombiningTransformer
            , typename U = std::remove_const_t<typename fhg::util::return_type<CombiningTransformer>::first_type>
            , typename B = typename fhg::util::return_type<CombiningTransformer>::second_type
            , typename = is_combining_transformer<CombiningTransformer, U, B>
            >
      Forest<U, B> upward_combine_transform (CombiningTransformer) const;

    //! unordered transform
    template<typename F, typename U, typename B>
      using is_unordered_transformer = std::enable_if_t
      < fhg::util::is_callable
          <F, forest::Node<U, B> (forest::Node<T, A> const&)>{}
      >;

    template< typename UnorderedTransformer
            , typename U = std::remove_const_t<typename fhg::util::return_type<UnorderedTransformer>::first_type>
            , typename B = typename fhg::util::return_type<UnorderedTransformer>::second_type
            , typename = is_unordered_transformer<UnorderedTransformer, U, B>
            >
      Forest<U, B> unordered_transform (UnorderedTransformer) const;

    //! \todo: iterate
    using Relation = std::unordered_map<T, Children>;
    using Annotations = std::unordered_map<T, A>;

  private:
    Relation _suc;
    Relation _pre;
    Annotations _annotations;

    bool is_unknown (T const&) const;
    T const& assert_is_known (T const&) const;

    friend boost::serialization::access;
    template<typename Archive>
      void serialize (Archive&, unsigned int);
  };
}

#include <gspc/Forest.ipp>
