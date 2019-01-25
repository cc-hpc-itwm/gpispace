#include <boost/format.hpp>

#include <util-generic/print_container.hpp>

#include <gspc/exception.hpp>

namespace gspc
{
  namespace detail
  {
    namespace StencilCache
    {
      template<typename Output, typename Counter>
        void InputEntry<Output, Counter>::increment()
      {
        _references.increment();
      }

      template<typename Output, typename Counter>
        bool InputEntry<Output, Counter>::free()
      {
        auto const not_in_use (_references.decrement());

        if (not_in_use && !_waiting.empty())
        {
          INCONSISTENCY
            ( boost::format ("InputEntry::free: orphan waiters %1%")
            % fhg::util::print_container ("{", ", ", "}", _waiting)
            );
        }

        return not_in_use;
      }

      template<typename Output, typename Counter>
        std::unordered_set<Output> const&
          InputEntry<Output, Counter>::waiting() const
      {
        return _waiting;
      }

      template<typename Output, typename Counter>
        void InputEntry<Output, Counter>::triggers (Output o)
      {
        if (!_waiting.emplace (std::move (o)).second)
        {
          INVALID_ARGUMENT ("Duplicate");
        }
        //! \todo throw INVALID_ARGUMENT if too many waiters (would
        //! lead to orhpans later though)
      }

      template<typename Output, typename Counter>
        void InputEntry<Output, Counter>::prepared()
      {
        _waiting.clear();
      }
    }
  }
}
