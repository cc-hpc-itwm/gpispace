#include <boost/format.hpp>

#include <exception>
#include <iterator>
#include <stdexcept>

namespace gspc
{
  template<typename T, typename ID>
    void threadsafe_interruptible_queue_with_remove<T, ID>::interrupt()
  {
    std::lock_guard<std::mutex> const lock (_guard);
    _interrupted = true;
    _elements_added_or_interrupted.notify_all();
  }

  template<typename T, typename ID>
    void threadsafe_interruptible_queue_with_remove<T, ID>::push (T x, ID id)
  try
  {
    std::lock_guard<std::mutex> const lock (_guard);

    if (!_positions.emplace (id, _before).second)
    {
      throw std::invalid_argument ("Duplicate.");
    }

    _before = _elements.insert_after (_before, std::make_pair (x, id));

    _elements_added_or_interrupted.notify_one();
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
        ( ( boost::format
              ("threadsafe_interruptible_queue_with_remove::push (id %1%)")
          % id
          ).str()
        )
      );
  }

  template<typename T, typename ID>
    std::pair<T, ID> threadsafe_interruptible_queue_with_remove<T, ID>::pop()
  try
  {
    std::unique_lock<std::mutex> lock (_guard);

    _elements_added_or_interrupted.wait
      (lock, [this] { return !_elements.empty() || _interrupted; });

    if (_interrupted)
    {
      throw Interrupted();
    }

    return *do_remove (_elements.front().second);
  }
  catch (Interrupted const&)
  {
    throw;
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
          ("threadsafe_interruptible_queue_with_remove::pop()")
      );
  }

  template<typename T, typename ID>
    boost::optional<std::pair<T, ID>>
      threadsafe_interruptible_queue_with_remove<T, ID>::remove (ID id)
  try
  {
    std::lock_guard<std::mutex> const lock (_guard);

    return do_remove (id);
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::runtime_error
          ( ( boost::format
                ("threadsafe_interruptible_queue_with_remove::remove (id %1%)")
            % id
            ).str()
          )
      );
  }

  template<typename T, typename ID>
    boost::optional<std::pair<T, ID>>
      threadsafe_interruptible_queue_with_remove<T, ID>::do_remove (ID id) noexcept
  {
    auto position {_positions.find (id)};

    if (position == _positions.end())
    {
      return boost::none;
    }

    auto before {position->second};
    auto item (std::move (*std::next (before)));
    auto next {std::next (before, 2)};

    if (next != _elements.end())
    {
      _positions[next->second] = before;
    }
    else
    {
      _before = before;
    }

    _positions.erase (position);
    _elements.erase_after (before);

    return item;
  }
}
