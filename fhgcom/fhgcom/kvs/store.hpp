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
      template <typename Backend, typename Cache>
      class basic_store
      {
      public:
        typedef Backend backend_type;
        typedef Cache cache_type;
        typedef typename cache_type::size_type cache_size_type;

        explicit
        basic_store ( backend_type & backend
                    , const cache_size_type max_size
                    )
          : backend_(backend)
          , cache_ (max_size)
        {}

        template <typename Val>
        void put (typename cache_type::key_type const & k, Val v)
        {
          cache_.put (k, v);
          backend_.put (k, v);
        }

        template <typename T>
        T get (typename cache_type::key_type const & k) const
        {
          try
          {
            // TODO: check timestamp?
            return cache_.template get<T>(k);
          }
          catch (std::exception const &)
          {
            T v = backend_.template get<T>(k);
            cache_.put (k,v);
            return v;
          }
        }

        typename cache_type::value_type
        get (typename cache_type::key_type const & k) const
        {
          try
          {
            return cache_.get(k);
          }
          catch (std::exception const &)
          {
            typename cache_type::value_type v (backend_.get(k));
            cache_.put (k,v);
            return v;
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
        backend_type & backend_;
        mutable cache_type cache_;
      };

      template <typename Val, typename B, typename C>
      inline void put (basic_store<B,C> & s, std::string const & k, Val v)
      {
        s.put (k, v);
      }

      template <typename Val, typename B, typename C>
      inline Val get (basic_store<B,C> const & s, std::string const & k)
      {
        return s.template get<Val>(k);
      }

      template <typename B, typename C>
      inline typename basic_store<B,C>::cache_type::value_type const &
      get (basic_store<B,C> const & s, std::string const & k)
      {
        return s.get(k);
      }
    }
  }
}

#endif
