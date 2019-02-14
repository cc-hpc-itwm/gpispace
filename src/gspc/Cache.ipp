#include <boost/format.hpp>

#include <exception>
#include <stdexcept>
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

  TEMPLATE struct CACHE::Entry
  {
    Entry (ID);

    ID id() const;

    bool remembered() const;
    void remember();

    bool in_use() const;
    bool is_first_use();
    bool is_last_use();

  private:
    ID _id;
    bool _remembered;
    detail::References<Counter> _references;
  };

  TEMPLATE CACHE::Cache (ID n)
    : _max {std::move (n)}
  {}

  TEMPLATE void CACHE::interrupt()
  {
    std::lock_guard<std::mutex> const _ {_guard};

    _interrupted = true;

    _grown_or_interrupted.notify_all();
  }

#define RETHROW_INTERRUPTED_OR_NEST(fun_)                               \
  catch (interrupted)                                                   \
  {                                                                     \
    throw;                                                              \
  }                                                                     \
  catch (...)                                                           \
  {                                                                     \
    std::throw_with_nested                                              \
      ( std::runtime_error                                              \
          (str (boost::format ("Cache::" fun_ " (%1%)") % value))       \
      );                                                                \
  }

  TEMPLATE typename CACHE::Allocation CACHE::alloc (T value)
  try
  {
    std::unique_lock<std::mutex> lock {_guard};

    return _alloc (value, lock);
  }
  RETHROW_INTERRUPTED_OR_NEST ("alloc")

  TEMPLATE void CACHE::free (T value)
  try
  {
    std::lock_guard<std::mutex> const _ {_guard};

    return _free (value, _);
  }
  RETHROW_INTERRUPTED_OR_NEST ("free")

  TEMPLATE void CACHE::remember (T value)
  try
  {
    std::lock_guard<std::mutex> const _ {_guard};

    return _remember (value, _);
  }
  RETHROW_INTERRUPTED_OR_NEST ("remember")

  TEMPLATE bool CACHE::forget (T value)
  try
  {
    std::lock_guard<std::mutex> const _ {_guard};

    return _forget (value, _);
  }
  RETHROW_INTERRUPTED_OR_NEST ("forget")

#undef RETHROW_INTERRUPTED_OR_NEST

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
        throw std::logic_error ("Unknown and known (next)");
      }

      return Allocation {id, Allocation::Empty};
    }

    if (!_empty.empty())
    {
      auto id (_empty.pop());

      if (!_known.emplace (value, id).second)
      {
        throw std::logic_error ("Unknown and known (empty)");
      }

      return Allocation {id, Allocation::Empty};
    }

    if (!_unused.empty())
    {
      auto to_evict (_unused.pop());

      auto known (_known.find (to_evict));

      if (known == _known.end())
      {
        throw std::logic_error
          (str (boost::format ("Unused and unknown %1%") % to_evict));
      }

      auto id (known->second.id());

      _known.erase (known);

      if (!_known.emplace (value, id).second)
      {
        throw std::logic_error ("Unknown and known (unused)");
      }

      return Allocation {id, Allocation::Empty};
    }

    throw std::logic_error ("Full");
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
      throw std::invalid_argument ("Unknown");
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
      throw std::invalid_argument ("Unknown");
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
      throw std::invalid_argument ("Not remembered");
    }

    if (entry.in_use())
    {
      throw std::invalid_argument ("In use");
    }

    _empty.push (entry.id());
    _known.erase (known);
    _unused.erase (value);

    return true;
  }


  TEMPLATE CACHE::Entry::Entry (ID id)
    : _id {std::move (id)}
    , _remembered {false}
    , _references{}
  {
    _references.increment();
  }

  TEMPLATE ID CACHE::Entry::id() const
  {
    return _id;
  }

  TEMPLATE bool CACHE::Entry::remembered() const
  {
    return _remembered;
  }

  TEMPLATE void CACHE::Entry::remember()
  {
    if (remembered())
    {
      throw std::logic_error ("Entry::remember: Already remembered");
    }

    _remembered = true;
  }

  TEMPLATE bool CACHE::Entry::in_use() const
  {
    return _references.in_use();
  }

  TEMPLATE bool CACHE::Entry::is_first_use()
  {
    if (_references.increment())
    {
      if (!_remembered)
      {
        throw std::logic_error ("Not in use and not remembered");
      }

      return true;
    }

    return false;
  }

  TEMPLATE bool CACHE::Entry::is_last_use()
  {
    return _references.decrement();
  }

#undef CACHE
#undef TEMPLATE
}
