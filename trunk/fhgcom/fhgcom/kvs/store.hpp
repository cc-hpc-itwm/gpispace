#ifndef FHG_COM_KVS_STORE_HPP
#define FHG_COM_KVS_STORE_HPP 1

#include <string>
#include <fhgcom/kvs/cache.hpp>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      template <typename Cache>
      class basic_store
      {
      public:
        typedef Cache cache_type;
        typedef typename cache_type::size_type cache_size_type;

        explicit
        basic_store (const cache_size_type max_size)
          : cache_ (max_size)
        {}

        template <typename Val>
        void put (typename cache_type::key_type const & k, Val v)
        {
          cache_.put (k, v);

          // TODO: make network communication
        }

        template <typename T>
        T get (typename cache_type::key_type const & k) const
        {
          try
          {
            return cache_.get<T>(k);
          }
          catch (std::exception const &)
          {
            // TODO: make network communication
            // cache_.put (k, v);
            throw;
          }
        }

        typename cache_type::value_type const &
        get (typename cache_type::key_type const & k) const
        {
          try
          {
            return cache_.get(k);
          }
          catch (std::exception const &)
          {
            // TODO: make network communication
            // cache_.put (k, v);
            throw;
          }
        }

        cache_size_type cache_size (void) const
        {
          return cache_.max_size();
        }
        void cache_size (cache_size_type s)
        {
          return cache_.max_size(s);
        }

        cache_size_type num_cached (void) const
        {
          return cache_.size();
        }
      private:
        mutable cache_type cache_;
      };

      typedef basic_store<detail::cache> store;

      template <typename Val>
      inline void put (store & s, std::string const & k, Val v)
      {
        s.put (k, v);
      }

      template <typename Val>
      inline Val get (store const & s, std::string const & k)
      {
        return s.get<Val>(k);
      }

      inline store::cache_type::value_type const &
      get (store const & s, std::string const & k)
      {
        return s.get(k);
      }
    }
  }
}

#endif
