#include <mmgr/heap.hpp>

#include <algorithm>

namespace gspc
{
  namespace mmgr
  {
    namespace exception
    {
      heap_empty::heap_empty (std::string const& function)
        : std::logic_error (function + ": heap is emptry")
      {}
    }

    void heap::insert (Offset_t offset)
    {
      _.emplace_back (offset);

      std::push_heap (_.begin(), _.end(), std::greater<Offset_t>());
    }
    std::vector<Offset_t>::size_type heap::size() const
    {
      return _.size();
    }
    Offset_t heap::min() const
    {
      if (_.begin() == _.end())
      {
        throw exception::heap_empty ("min");
      }

      return _.front();
    }
    void heap::delete_min()
    {
      if (_.begin() == _.end())
      {
        throw exception::heap_empty ("delete_min");
      }

      std::pop_heap (_.begin(), _.end(), std::greater<Offset_t>());

      _.pop_back();
    }
  }
}
