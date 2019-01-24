#include <gspc/detail/StencilCache/InputEntry.hpp>
#include <gspc/detail/StencilCache/OutputEntry.hpp>
#include <gspc/exception.hpp>

#include <util-generic/print_container.hpp>

#include <boost/format.hpp>

#include <utility>

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


  TEMPLATE STENCILCACHE::StencilCache
    ( InputEntries inputs
    , Prepare prepare
    , Ready ready
    , Slot slot
    )
      : Base {std::move (slot)}
      , _prepare {std::move (prepare)}
      , _ready {std::move (ready)}
      , _inputs {std::move (inputs)}
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
      INVALID_ARGUMENT ("Duplicate");
    }

    for (auto i : is)
    {
      auto const allocation {Base::_alloc (i, lock)};

      if (allocation.state != Base::Allocation::Remembered)
      {
        auto input {_inputs.find (i)};

        if (input == _inputs.end())
        {
          INVALID_ARGUMENT (boost::format ("Unknown %1%") % i);
        }

        auto& input_entry {input->second};

        input_entry.triggers (o);
      }

      {
        auto output {_outputs.find (o)};

        if (output == _outputs.end())
        {
          INCONSISTENCY ("Unknown");
        }

        auto& output_entry {output->second};

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

    return;
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

    auto input {_inputs.find (i)};

    if (input == _inputs.end())
    {
      INVALID_ARGUMENT ("Unknown");
    }

    auto& input_entry {input->second};

    for (auto o : input_entry.waiting())
    {
      auto output {_outputs.find (o)};

      if (output == _outputs.end())
      {
        INCONSISTENCY (boost::format ("Missing output %1%") % o);
      }

      auto& output_entry {output->second};

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

    auto input {_inputs.find (i)};

    if (input == _inputs.end())
    {
      INVALID_ARGUMENT ("Unknown");
    }

    Base::_free (i, _);

    auto& input_entry {input->second};

    if (input_entry.free())
    {
      _inputs.erase (input);

      Base::_forget (i, _);
    }

    return;
  }
  catch (typename STENCILCACHE::Base::interrupted)
  {
    throw interrupted{};
  }
  catch (...)
  {
    NESTED_RUNTIME_ERROR (boost::format ("StencilCache::free (%1%)") % i);
  }

#undef STENCILCACHE
#undef TEMPLATE
}
