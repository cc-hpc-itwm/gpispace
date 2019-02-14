#include <gspc/exception.hpp>
#include <gspc/detail/Cache/Entry.hpp>

#include <boost/format.hpp>

#include <utility>

namespace gspc
{
#define TEMPLATE template< typename ID                                  \
                         , typename T                                   \
                         , typename Counter                             \
                         , template<typename> class UniqEmpty           \
                         , template<typename> class UniqUnused          \
                         >
#define CACHE Cache<ID, T, Counter, UniqEmpty, UniqUnused>

  TEMPLATE CACHE::Cache (ID n)
    : _max {std::move (n)}
  {}

  TEMPLATE void CACHE::interrupt()
  {
    std::lock_guard<std::mutex> const _ {_guard};

    _interrupted = true;

    _grown_or_interrupted.notify_all();
  }

  TEMPLATE typename CACHE::Allocation CACHE::alloc (T value)
  try
  {
    std::unique_lock<std::mutex> lock {_guard};

    return _alloc (value, lock);
  }
  catch (interrupted)
  {
    throw;
  }
  catch (...)
  {
    NESTED_RUNTIME_ERROR (boost::format ("Cache::alloc (%1%)") % value);
  }

  TEMPLATE void CACHE::free (T value)
  try
  {
    std::lock_guard<std::mutex> const _ {_guard};

    return _free (value, _);
  }
  catch (interrupted)
  {
    throw;
  }
  catch (...)
  {
    NESTED_RUNTIME_ERROR (boost::format ("Cache::free (%1%)") % value);
  }

  TEMPLATE void CACHE::remember (T value)
  try
  {
    std::lock_guard<std::mutex> const _ {_guard};

    return _remember (value, _);
  }
  catch (interrupted)
  {
    throw;
  }
  catch (...)
  {
    NESTED_RUNTIME_ERROR (boost::format ("Cache::remember (%1%)") % value);
  }

  TEMPLATE bool CACHE::forget (T value)
  try
  {
    std::lock_guard<std::mutex> const _ {_guard};

    return _forget (value, _);
  }
  catch (interrupted)
  {
    throw;
  }
  catch (...)
  {
    NESTED_RUNTIME_ERROR (boost::format ("Cache::forget (%1%)") % value);
  }

  TEMPLATE
    typename CACHE::Allocation
      CACHE::_alloc
        ( T value
        , std::unique_lock<std::mutex>& lock
        )
  {
    if (!_interrupted)
    {
      auto known (_known.find (value));

      if (known != _known.end())
      {
        auto& entry (known->second);

        if (entry.is_first_use())
        {
          _unused.erase (value);

          return Allocation {entry.id(), Allocation::Remembered};
        }

        return Allocation { entry.id()
                          , entry.remembered()
                          ? Allocation::Remembered
                          : Allocation::Assigned
                          };
      }
    }

    _grown_or_interrupted.wait
      ( lock
      , [&]
        {
          return _interrupted
            || _next < _max
            || !_empty.empty()
            || !_unused.empty()
            ;
        }
      );

    if (_interrupted)
    {
      throw interrupted();
    }

    if (_next < _max)
    {
      auto id (_next++);

      if (!_known.emplace (value, id).second)
      {
        INCONSISTENCY ("Unknown and known (next)");
      }

      return Allocation {id, Allocation::Empty};
    }

    if (!_empty.empty())
    {
      auto id (_empty.pop());

      if (!_known.emplace (value, id).second)
      {
        INCONSISTENCY ("Unknown and known (empty)");
      }

      return Allocation {id, Allocation::Empty};
    }

    if (!_unused.empty())
    {
      auto to_evict (_unused.pop());

      auto known (_known.find (to_evict));

      if (known == _known.end())
      {
        INCONSISTENCY (boost::format ("Unused and unknown %1%") % to_evict);
      }

      auto id (known->second.id());

      _known.erase (known);

      if (!_known.emplace (value, id).second)
      {
        INCONSISTENCY ("Unknown and known (unused)");
      }

      return Allocation {id, Allocation::Empty};
    }

    INCONSISTENCY ("Full");
  }

  TEMPLATE
    void CACHE::_free
      ( T value
      , std::lock_guard<std::mutex> const&
      )
  {
    if (_interrupted)
    {
      throw interrupted();
    }

    auto known (_known.find (value));

    if (known == _known.end())
    {
      INVALID_ARGUMENT ("Unknown");
    }

    auto& entry (known->second);

    if (entry.is_last_use())
    {
      if (!entry.remembered())
      {
        _empty.push (entry.id());
        _known.erase (known);
      }
      else
      {
        _unused.push (value);
      }

      _grown_or_interrupted.notify_one();
    }
  }

  TEMPLATE
    void CACHE::_remember
      ( T value
      , std::lock_guard<std::mutex> const&
      )
  {
    if (_interrupted)
    {
      throw interrupted();
    }

    auto known (_known.find (value));

    if (known == _known.end())
    {
      INVALID_ARGUMENT ("Unknown");
    }

    auto& entry (known->second);

    return entry.remember();
  }

  TEMPLATE
    bool CACHE::_forget
      ( T value
      , std::lock_guard<std::mutex> const&
      )
  {
    if (_interrupted)
    {
      throw interrupted();
    }

    auto known (_known.find (value));

    if (known == _known.end())
    {
      return false;
    }

    auto const& entry (known->second);

    if (!entry.remembered())
    {
      INVALID_ARGUMENT ("Not remembered");
    }

    if (entry.in_use())
    {
      INVALID_ARGUMENT ("In use");
    }

    _empty.push (entry.id());
    _known.erase (known);
    _unused.erase (value);

    return true;
  }

#undef CACHE
#undef TEMPLATE
}
