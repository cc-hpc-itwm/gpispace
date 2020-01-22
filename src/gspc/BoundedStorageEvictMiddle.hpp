#pragma once

#include <gspc/StorageWithEvict.hpp>

namespace gspc
{
  template<typename ID, typename T>
    struct BoundedStorageEvictMiddle : StorageWithEvict<ID, T>
  {
    using Base = StorageWithEvict<ID, T>;

    BoundedStorageEvictMiddle
      (typename Base::Storage::size_type maximum_number_of_elements);

    template<typename... Args> T& at_or_construct (ID, Args&&...);
    template<typename Create> T& at_or_create (ID, Create&&);

  private:
    typename Base::Storage::size_type _maximum_number_of_elements;

    void maybe_evict_some (typename Base::Storage&) const;
  };
}

#include <gspc/BoundedStorageEvictMiddle.ipp>
