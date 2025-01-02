// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/warning.hpp>

#include <algorithm>
#include <future>
#include <iterator>
#include <list>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace detail
    {
      //! \note std::next is explicitly defined as Distance ==
      //! It::distance_type, while std::advance is not, so we do that
      //! ourself to avoid narrowing
      template<typename It, typename Distance>
        It next (It it, Distance distance)
      {
        std::advance (it, distance);
        return it;
      }
    }

    template<typename Fun, typename It>
      void blocked ( It begin, It end
                   , ::boost::optional<std::size_t> const& block_size
                   , Fun&& fun
                   )
    {
      auto size
        ( fhg::util::suppress_warning::sign_conversion<std::size_t>
            ( std::distance (begin, end)
            , "begin < end"
            )
        );

      if (size > 0 && !!block_size && !block_size.get())
      {
        throw std::invalid_argument ("blocksize must be positive");
      }

      while (begin != end)
      {
        std::size_t const count
          (std::min (size, block_size.get_value_or (size)));

        fun (begin, detail::next (begin, count));

        std::advance (begin, count);
        size -= count;
      }
    }

    template<typename Key, typename Result, typename Fun, typename Container>
      std::pair< std::unordered_map<Key, Result>
               , std::unordered_map<Key, std::exception_ptr>
               >
        blocked_async_with_results
          ( Container const& container
          , ::boost::optional<std::size_t> const& block_size
          , std::function<Key (typename Container::const_iterator::value_type const&)>
              const& key
          , Fun&& fun
          )
    {
      std::unordered_map<Key, std::exception_ptr> failures;
      std::unordered_map<Key, Result> successes;

      blocked
        ( container.cbegin(), container.cend(), block_size
        , [fun, key, &failures, &successes]
            ( typename Container::const_iterator element
            , typename Container::const_iterator end
            )
          {
            std::list<std::pair< typename Container::const_iterator
                               , std::future<Result>
                               >
                     > executions;

            while (element != end)
            {
              executions.emplace_back
                ( std::make_pair
                  ( element
                  , std::async (std::launch::async, fun, *element)
                  )
                );

              ++element;
            }

            for (auto& execution : executions)
            {
              try
              {
                successes.emplace ( key (*execution.first)
                                  , execution.second.get()
                                  );
              }
              catch (...)
              {
                failures.emplace
                  (key (*execution.first), std::current_exception());
              }
            }
          }
        );

      return {std::move (successes), std::move (failures)};
    }

    template<typename Key, typename Fun, typename Container>
      std::pair< std::unordered_set<Key>
               , std::unordered_map<Key, std::exception_ptr>
               >
        blocked_async
          ( Container const& container
          , ::boost::optional<std::size_t> const& block_size
          , std::function<Key (typename Container::const_iterator::value_type const&)>
              const& key
          , Fun&& fun
          )
    {
      std::unordered_map<Key, std::exception_ptr> failures;
      std::unordered_set<Key> successes;

      blocked
        ( container.cbegin(), container.cend(), block_size
        , [fun, key, &failures, &successes]
            ( typename Container::const_iterator element
            , typename Container::const_iterator end
            )
          {
            std::list<std::pair< typename Container::const_iterator
                               , std::future<void>
                               >
                     > executions;

            while (element != end)
            {
              executions.emplace_back
                ( std::make_pair
                  ( element
                  , std::async (std::launch::async, fun, *element)
                  )
                );

              ++element;
            }

            for (auto& execution : executions)
            {
              try
              {
                execution.second.get();

                successes.emplace (key (*execution.first));
              }
              catch (...)
              {
                failures.emplace
                  (key (*execution.first), std::current_exception());
              }
            }
          }
        );

      return {std::move (successes), std::move (failures)};
    }
  }
}
