#include <gspc/exception.hpp>

#include <utility>

namespace gspc
{
  namespace detail
  {
    namespace Cache
    {
      template<typename ID, typename Counter>
        Entry<ID, Counter>::Entry (ID id)
          : _id {std::move (id)}
          , _remembered {false}
          , _references{}
      {
        _references.increment();
      }

      template<typename ID, typename Counter>
        ID Entry<ID, Counter>::id() const
      {
        return _id;
      }

      template<typename ID, typename Counter>
        bool Entry<ID, Counter>::remembered() const
      {
        return _remembered;
      }

      template<typename ID, typename Counter>
        void Entry<ID, Counter>::remember()
      {
        if (remembered())
        {
          LOGIC_ERROR ("Entry::remember: Already remembered");
        }

        _remembered = true;

        return;
      }

      template<typename ID, typename Counter>
        bool Entry<ID, Counter>::in_use() const
      {
        return _references.in_use();
      }

      template<typename ID, typename Counter>
        bool Entry<ID, Counter>::is_first_use()
      {
        if (_references.increment())
        {
          if (!_remembered)
          {
            INCONSISTENCY ("Not in use and not remembered");
          }

          return true;
        }

        return false;
      }

      template<typename ID, typename Counter>
        bool Entry<ID, Counter>::is_last_use()
      {
        return _references.decrement();
      }
    }
  }
}
