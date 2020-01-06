#pragma once

#include <functional>
#include <iosfwd>
#include <vector>

namespace gspc
{
  // no cycle, no diamond
  template<typename T>
    struct Tree
  {
    // create a leaf
    Tree (T);

    T const& value() const noexcept;

    void add_child (Tree<T>);

    void for_each_child (std::function<void (Tree<T> const&)> const&) const;

    // for more see rat/Forest.hpp
  private:
    T _value;
    std::vector<Tree<T>> _children;
  };

  template<typename T>
    std::ostream& operator<< (std::ostream&, Tree<T> const&);

  //! apply \a function to each node in \a tree and throw whenever the
  //! callback function throws
  template<typename T, typename U>
    Tree<U> apply
    ( Tree<T> const& tree
    , std::function<U (T const&)> const& function
    );

  template<typename T>
    using Forest = std::vector<Tree<T>>;
}

#include <gspc/Tree.ipp>
