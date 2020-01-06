#include <algorithm>
#include <utility>

namespace gspc
{
  template<typename T>
    Tree<T>::Tree (T value)
      : _value {std::move (value)}
  {}

  template<typename T>
    T const& Tree<T>::value() const noexcept
  {
    return _value;
  }

  template<typename T>
    void Tree<T>::add_child (Tree<T> tree)
  {
    _children.emplace_back (std::move (tree));
  }

  template<typename T>
    void Tree<T>::for_each_child
    ( std::function<void (Tree<T> const&)> const& function
    ) const
  {
    std::for_each (_children.begin(), _children.end(), function);
  }

  template<typename T, typename U>
    Tree<U> apply
    ( Tree<T> const& tree
    , std::function<U (T const&)> const& function
    )
  {
    Tree<U> result (function (tree.value()));

    tree.for_each_child
      ( [&] (Tree<T> const& child)
        {
          result.add_child (apply (child, function));

          return true;
        }
      );

    return result;
  }

  namespace detail
  {
    template<typename T>
      std::ostream& print (std::ostream& os, Tree<T> const& tree, unsigned d)
    {
      os << std::string (d << 1, ' ') << tree.value() << '\n';

      tree.for_each_child
        ( [&] (Tree<T> const& child)
          {
            print (os, child, d + 1);
          }
        );

      return os;
    }
  }

  template<typename T>
    std::ostream& operator<< (std::ostream& os, Tree<T> const& tree)
  {
    return detail::print<T> (os, tree, 0);
  }
}
