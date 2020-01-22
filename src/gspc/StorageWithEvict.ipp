#include <utility>

namespace gspc
{
#define GET_AND_MAYBE_EVICT()                   \
  auto element (_elements.find (id));           \
                                                \
  if (element != _elements.end())               \
  {                                             \
    return element->second;                     \
  }                                             \
                                                \
  std::forward<Evict> (evict) (_elements)


  template<typename ID, typename T>
    template< typename Evict, typename... Args, typename, typename>
      T& StorageWithEvict<ID,T>::at_or_construct
        (ID id, Evict&& evict, Args&&... args)
  {
    GET_AND_MAYBE_EVICT();

    return _elements.emplace ( std::piecewise_construct
                             , std::forward_as_tuple (id)
                             , std::forward_as_tuple (args...)
                             ).first->second;
  }

  template<typename ID, typename T>
    template< typename Evict, typename Create, typename, typename>
      T& StorageWithEvict<ID,T>::at_or_create
        (ID id, Evict&& evict, Create&& create)
  {
    GET_AND_MAYBE_EVICT();

    std::forward<Evict> (evict) (_elements);

    return _elements.emplace ( std::move (id)
                             , std::forward<Create> (create)()
                             ).first->second;
  }

#undef GET_AND_MAYBE_EVICT
}
