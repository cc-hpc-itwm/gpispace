#include <util-generic/this_bound_mem_fn.hpp>

#include <iterator>
#include <utility>

namespace gspc
{
  template<typename ID, typename T>
    BoundedStorageEvictMiddle<ID, T>::BoundedStorageEvictMiddle
      (typename Base::Storage::size_type maximum_number_of_elements)
    : _maximum_number_of_elements (maximum_number_of_elements)
  {}

  template<typename ID, typename T>
    void BoundedStorageEvictMiddle<ID, T>::maybe_evict_some
      (typename Base::Storage& elements) const
  {
    if (! (elements.size() < _maximum_number_of_elements))
    {
      elements.erase
        (std::next (elements.begin(), _maximum_number_of_elements / 2));
    }
  }

  template<typename ID, typename T>
    template<typename... Args>
      T& BoundedStorageEvictMiddle<ID, T>::at_or_construct
        (ID id, Args&&... args)
  {
    return Base::at_or_construct
      ( std::move (id)
      , fhg::util::bind_this
          (this, &BoundedStorageEvictMiddle<ID, T>::maybe_evict_some)
      , std::forward<Args> (args)...
      );
  }
  template<typename ID, typename T>
    template<typename Create>
      T& BoundedStorageEvictMiddle<ID, T>::at_or_create
        (ID id, Create&& create)
  {
    return Base::at_or_create
      ( std::move (id)
      , fhg::util::bind_this
          (this, &BoundedStorageEvictMiddle<ID, T>::maybe_evict_some)
      , std::forward<Create> (create)
      );
  }
}
