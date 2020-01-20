#pragma once

#include <util-generic/callable_signature.hpp>
#include <util-generic/ostream/modifier.hpp>

#include <boost/blank.hpp>
#include <boost/serialization/access.hpp>

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
    using Node = forest::Node<T, A>;
    using Children = std::unordered_set<T>;
    using Relation = std::unordered_map<T, Children>;
    using Annotations = std::unordered_map<T, A>;

    Forest() = default;
    Forest (Forest const&) = default;       //! \todo rpc requires copy
    Forest (Forest&&) = default;
    Forest& operator= (Forest const&) = delete;
    Forest& operator= (Forest&&) = delete;
    ~Forest() = default;

    //! 1 throw when duplicate x
    //! 2 throw when any_of (cs: unknown)
    //!   1 + 2 imply: throw when cycle
    //! 3 throw when diamond
    forest::Node<T, A> const& insert (T x, A a, Children cs);
    forest::Node<T, A> const& insert (forest::Node<T, A>, Children);

    //! throw when unknown
    //! throw when not leaf/root
    //! disconnects and removes node
    Node remove_leaf (T);
    Node remove_root (T);

    template<typename F>
      using is_predicate =
        fhg::util::is_callable<F, bool (forest::Node<T, A> const&)>;

    //! downward iterate, stop subtree iteration if not removed
    //! post: \all root: !pred(root)
    template<typename Pred, typename = std::enable_if_t<is_predicate<Pred>{}>>
      Forest remove_root_if (Pred&&) &&;

    // traverse downwards/upwards:
    // callback called once for the root and once for each transitive child
    // throw when root is unknown
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

    //! go up for each transitive down-child
    template< typename Callback
            , typename = std::enable_if_t<is_callback<Callback>{}>
            >
      void down_up (T, Callback&&) const;

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

    //! throw if node already known
    void merge (Forest<T, A> other);
    //! no check for duplicates
    void UNSAFE_merge (Forest<T, A> other);

    //! multiway split:
    //! - throws when connected component has more than one key
    //! - call split_key function at most once for each node
    template<typename F, typename Key>
      using is_split_function =
        fhg::util::is_callable<F, Key (forest::Node<T, A> const&)>;

    template< typename SplitKey
            , typename Key = fhg::util::return_type<SplitKey>
            , typename = std::enable_if_t<is_split_function<SplitKey, Key>{}>
            >
      std::unordered_map<Key, Forest<T, A>> multiway_split (SplitKey&&) const;

    //! \todo: iterate

    //! traverse children before parents
    template< typename Callback
            , typename = std::enable_if<is_callback<Callback>{}>
            >
      void upward_apply (Callback&&) const;

    //! unspecified order
    template< typename Callback
            , typename = std::enable_if_t<is_callback<Callback>{}>
            >
      void for_each_node (Callback&&) const;

    template< typename Callback
            , typename = std::enable_if_t<is_callback<Callback>{}>
            >
      void for_each_leaf (Callback&&) const;

    //! turn all arrows
    Forest<T, A> mirrored() const;

  protected:
    Relation _suc;
    Relation _pre;
    Annotations _annotations;

    Forest (Annotations, Relation, Relation);

    bool is_unknown (T const&) const;
    bool is_leaf (T const&) const;
    bool is_root (T const&) const;

    T const& assert_is_known (T const&) const;
    T const& assert_is_root (T const&) const;
    T const& assert_is_leaf (T const&) const;

    template< typename Callback
            , typename = std::enable_if_t<is_callback<Callback>{}>
            >
      void for_each_root (Callback&&) const;

    friend boost::serialization::access;
    template<typename Archive>
      void serialize (Archive&, unsigned int);
  };

  namespace unique_forest
  {
    template<typename A, typename T = std::uint64_t>
      using Node = forest::Node<T, A>;
  }

  template<typename T, typename A> struct ToDot;

  //! \todo merge Forest and UniqueForest:
  //! allow for client id. if not provided, then generate, avoid tuple
  //! like: Forest<Annotations&&..., typename T = std::uint64_t>
  template<typename A, typename T = std::uint64_t>
    class UniqueForest : private Forest<T, A>
  {
  public:
    using Forest<T, A>::Forest;

    UniqueForest (Forest<T, A>);

    T insert (A, typename Forest<T, A>::Children const&);

    //! \todo: free client from key management (e.g. resource.first)
    using Forest<T, A>::upward_combine_transform;
    using Forest<T, A>::remove_root_if;
    using Forest<T, A>::unordered_transform;

    template<typename Archive>
      void serialize (Archive&, unsigned int);

  private:
    static_assert (std::is_integral<T>{}, "UniqueForest: Key not integral.");

    friend struct ToDot<T, A>;

    T _next_key {0};
  };

  template<typename T, typename A = boost::blank>
    struct ToDot : public fhg::util::ostream::modifier
  {
    ToDot (Forest<T, A> const&);
    ToDot (UniqueForest<A, T> const&);
    virtual std::ostream& operator() (std::ostream&) const override;
  private:
    Forest<T, A> const& _forest;
  };
}

#include <gspc/Forest.ipp>
