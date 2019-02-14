#include <util-generic/print_container.hpp>

#include <boost/format.hpp>

#include <exception>
#include <mutex>
#include <stdexcept>

namespace gspc
{
#define TEMPLATE template< typename Output                              \
                         , typename Input                               \
                         , typename Slot                                \
                         , typename Counter                             \
                         , template<typename> class UniqEmpty           \
                         , template<typename> class UniqUnused          \
                         >
#define STENCILCACHE                                                    \
  StencilCache<Output, Input, Slot, Counter, UniqEmpty, UniqUnused>

#define NESTED_RUNTIME_ERROR(what...)                                   \
  std::throw_with_nested (std::runtime_error (str (what)))

  TEMPLATE struct STENCILCACHE::OutputEntry
  {
    using Assigned = std::pair<Slot, Input>;
    using Assignment = std::list<Assigned>;

    OutputEntry (Counter);

    bool prepared();
    Assignment const& assignment() const;
    void assigned (Slot, Input);

  private:
    bool all_inputs_prepared() const;

    Counter _count_down;
    Assignment _assignment;
  };

  TEMPLATE STENCILCACHE::StencilCache
    ( InputEntries inputs
    , Prepare prepare
    , Ready ready
    , Slot slot
    )
      : Base {std::move (slot)}
      , _prepare {std::move (prepare)}
      , _ready {std::move (ready)}
      , _inputs {inputs.begin(), inputs.end()}
  {}

  TEMPLATE void STENCILCACHE::interrupt()
  {
    return Base::interrupt();
  }

  TEMPLATE void STENCILCACHE::alloc (std::pair<Output, Inputs> ois)
  {
    return alloc (ois.first, ois.second);
  }

  TEMPLATE void STENCILCACHE::alloc (Output o, Inputs is)
  try
  {
    std::unique_lock<std::mutex> lock {Base::_guard};

    if (Base::_interrupted)
    {
      throw interrupted();
    }

    if (!_outputs.emplace (o, is.size()).second)
    {
      throw std::invalid_argument ("Duplicate");
    }

    for (auto i : is)
    {
      auto const allocation (Base::_alloc (i, lock));

      if (allocation.state != Base::Allocation::Remembered)
      {
        auto input (_inputs.find (i));

        if (input == _inputs.end())
        {
          throw std::invalid_argument
            (str (boost::format ("Unknown %1%") % i));
        }

        auto& input_entry (input->second);

        input_entry.triggers (o);
      }

      {
        auto output (_outputs.find (o));

        if (output == _outputs.end())
        {
          throw std::logic_error ("Unknown");
        }

        auto& output_entry (output->second);

        output_entry.assigned (allocation.id, i);

        if (allocation.state == Base::Allocation::Remembered)
        {
          if (output_entry.prepared())
          {
            _ready (o, output_entry.assignment());

            _outputs.erase (output);
          }
        }
      }

      if (allocation.state == Base::Allocation::Empty)
      {
        _prepare (allocation.id, i);
      }
    }
  }
  catch (typename STENCILCACHE::Base::interrupted)
  {
    throw interrupted{};
  }
  catch (...)
  {
    NESTED_RUNTIME_ERROR
      ( boost::format ("StencilCache::alloc (%1%, %2%)")
      % o
      % fhg::util::print_container ("{", ", ", "}", is)
      );
  }

  TEMPLATE void STENCILCACHE::prepared (Input i)
  try
  {
    std::lock_guard<std::mutex> const _ {Base::_guard};

    if (Base::_interrupted)
    {
      throw interrupted();
    }

    auto input (_inputs.find (i));

    if (input == _inputs.end())
    {
      throw std::invalid_argument ("Unknown");
    }

    auto& input_entry (input->second);

    for (auto o : input_entry.waiting())
    {
      auto output (_outputs.find (o));

      if (output == _outputs.end())
      {
        throw std::logic_error (str (boost::format ("Missing output %1%") % o));
      }

      auto& output_entry (output->second);

      if (output_entry.prepared())
      {
        _ready (o, output_entry.assignment());

        _outputs.erase (output);
      }
    }

    Base::_remember (i, _);

    return input_entry.prepared();
  }
  catch (typename STENCILCACHE::Base::interrupted)
  {
    throw interrupted{};
  }
  catch (...)
  {
    NESTED_RUNTIME_ERROR (boost::format ("StencilCache::prepared (%1%)") % i);
  }

  TEMPLATE void STENCILCACHE::free (Input i)
  try
  {
    std::lock_guard<std::mutex> const _ {Base::_guard};

    if (Base::_interrupted)
    {
      throw interrupted();
    }

    auto input (_inputs.find (i));

    if (input == _inputs.end())
    {
      throw std::invalid_argument ("Unknown");
    }

    Base::_free (i, _);

    auto& input_entry (input->second);

    if (input_entry.free())
    {
      _inputs.erase (input);

      Base::_forget (i, _);
    }
  }
  catch (typename STENCILCACHE::Base::interrupted)
  {
    throw interrupted{};
  }
  catch (...)
  {
    NESTED_RUNTIME_ERROR (boost::format ("StencilCache::free (%1%)") % i);
  }

  TEMPLATE void STENCILCACHE::InputEntry::increment()
  {
    _references.increment();
  }

  TEMPLATE bool STENCILCACHE::InputEntry::free()
  {
    auto const not_in_use (InputEntry::_references.decrement());

    if (not_in_use && !_waiting.empty())
    {
      throw std::logic_error
        ( str ( boost::format ("InputEntry::free: orphan waiters %1%")
              % fhg::util::print_container ("{", ", ", "}", _waiting)
              )
        );
    }

    return not_in_use;
  }

  TEMPLATE std::unordered_set<Output> const&
    STENCILCACHE::InputEntry::waiting() const
  {
    return _waiting;
  }

  TEMPLATE void STENCILCACHE::InputEntry::triggers (Output o)
  {
    if (!_waiting.emplace (std::move (o)).second)
    {
      throw std::invalid_argument ("Duplicate");
    }
    //! \todo throw std::invalid_argument if too many waiters (would
    //! lead to orhpans later though)
  }

  TEMPLATE void STENCILCACHE::InputEntry::prepared()
  {
    _waiting.clear();
  }

  TEMPLATE STENCILCACHE::OutputEntry::OutputEntry (Counter counter)
    : _count_down {std::move (counter)}
  {
    if (all_inputs_prepared())
    {
      throw std::invalid_argument ("OutputEntry with no contributors");
    }
  }
  TEMPLATE bool STENCILCACHE::OutputEntry::prepared()
  {
    if (all_inputs_prepared())
    {
      throw std::logic_error
        ("OutputEntry::prepared() for already completed entry");
    }

    --_count_down;

    return all_inputs_prepared();
  }
  TEMPLATE bool STENCILCACHE::OutputEntry::all_inputs_prepared() const
  {
    return _count_down == 0;
  }
  TEMPLATE auto STENCILCACHE::OutputEntry::assignment() const
    -> Assignment const&
  {
    return _assignment;
  }
  TEMPLATE void STENCILCACHE::OutputEntry::assigned (Slot slot, Input i)
  {
    _assignment.emplace_back (Assigned {slot, i});
  }

#undef NESTED_RUNTIME_ERROR
#undef STENCILCACHE
#undef TEMPLATE
}
