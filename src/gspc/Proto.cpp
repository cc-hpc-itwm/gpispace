#include <gspc/Proto.hpp>

#include <gspc/Forest.hpp>

#include <exception>
#include <utility>

namespace gspc
{
  //! \todo think about combinators like this
  template<typename T, typename U>
    std::pair<Forest<T, U>, Forest<T, std::exception_ptr>>
      split (Forest<T, ErrorOr<U>> forest)
  {
    Forest<T, ErrorOr<U>> copy (forest);

    return
      { std::move (forest)
      . remove_root_if
        ( [] (forest::Node<T, ErrorOr<U>> const& node)
          {
            return !node.second;
          }
        )
      . unordered_transform
        ( [] (forest::Node<T, ErrorOr<U>> const& node)
          {
            return forest::Node<T, U> (node.first, node.second.value());
          }
       )
      , std::move (copy)
      . remove_root_if
        ( [] (forest::Node<T, ErrorOr<U>> const& node)
          {
            return node.second;
          }
        )
      . unordered_transform
        ( [] (forest::Node<T, ErrorOr<U>> const& node)
          {
            return forest::Node<T, std::exception_ptr>
              (node.first, node.second.error());
          }
       )
      };
  }
}
