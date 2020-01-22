#pragma once

#include <util-generic/callable_signature.hpp>

#include <type_traits>
#include <unordered_map>

namespace gspc
{
  template<typename ID, typename T>
    struct StorageWithEvict
  {
    using Storage = std::unordered_map<ID, T>;

    template<typename Evict> using is_evict =
      fhg::util::is_callable<Evict, void (Storage&)>;

    template< typename Evict
            , typename... Args
            , typename = std::enable_if_t<is_evict<Evict>{}>
            , typename = std::enable_if_t<std::is_constructible<T, Args...>{}>
            >
      T& at_or_construct (ID, Evict&&, Args&&...);

    template< typename Evict
            , typename Create
            , typename = std::enable_if_t<is_evict<Evict>{}>
            , typename = std::enable_if_t<fhg::util::is_callable<Create, T()>{}>
            >
      T& at_or_create (ID, Evict&&, Create&&);

  private:
    Storage _elements;
  };
}

#include <gspc/StorageWithEvict.ipp>
