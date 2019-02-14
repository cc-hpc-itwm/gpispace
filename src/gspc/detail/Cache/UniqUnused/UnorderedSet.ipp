#include <stdexcept>
#include <utility>

namespace gspc
{
  namespace detail
  {
    namespace Cache
    {
      namespace UniqUnused
      {
        template<typename T>
          bool UnorderedSet<T>::empty() const
        {
          return _.empty();
        }

        template<typename T>
          void UnorderedSet<T>::push (T x)
        {
          if (!_.emplace (std::move (x)).second)
          {
            throw std::invalid_argument ("UniqUnused::push: Duplicate.");
          }
        }

        template<typename T>
          T UnorderedSet<T>::pop()
        {
          auto x (*_.begin());
          _.erase (_.begin());

          return x;
        }

        template<typename T>
          void UnorderedSet<T>::erase (T x)
        {
          if (_.erase (x) != 1)
          {
            throw std::invalid_argument ("UniqUnused::erase: Unknown.");
          }
        }
      }
    }
  }
}
