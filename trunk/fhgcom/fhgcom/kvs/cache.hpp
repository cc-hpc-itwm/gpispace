#ifndef FHG_COM_KVS_CACHE_HPP
#define FHG_COM_KVS_CACHE_HPP 1

#include <fhglog/fhglog.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>

#include <string>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace detail
      {
        class cache
        {
        public:
          typedef std::size_t size_type;
          typedef std::string key_type;
          typedef std::string value_type;
          typedef std::pair<value_type, size_type> entry_type;
          typedef boost::unordered_map< key_type
                                      , entry_type
                                      > store_type;

          explicit
          cache (const size_type max_size)
            : max_size_ (max_size)
          {}

          template <typename Val>
          void put (key_type const & k, Val v)
          {
            if (max_size_)
            {
              while (store_.size() >= max_size_)
              {
                remove_old ();
              }
            }

            store_.insert
              (std::make_pair( k
                             , std::make_pair( boost::lexical_cast<value_type>(v)
                                             , size_type (0)
                                             )
                             )
              );
          }

          template <typename T>
          T get (key_type const & k) const
          {
            return boost::lexical_cast<T> (get(k));
          }

          std::string const & get(key_type const & k) const
          {
            typedef store_type::iterator item_it;
            item_it item (store_.find(k));
            if (item != store_.end())
            {
              ++item->second.second;
              return item->second.first;
            }
            else
            {
              // TODO: better exception
              throw std::runtime_error ("no_such: " + k);
            }
          }

          size_type size () const { return store_.size(); }
          size_type max_size () const { return max_size_; }
          void max_size (const size_type s) { max_size_ = s; }
        private:
          size_type remove_old ()
          {
            if (store_.empty())
            {
              return 0;
            }
            else
            {
              // TODO: use speed-up data structure
              typedef store_type::iterator item_iterator;
              item_iterator to_remove (store_.begin());

              for ( item_iterator it (store_.begin())
                  ; it != store_.end()
                  ; ++it
                  )
              {
                if (it->second.second > to_remove->second.second)
                {
                  to_remove = it;
                }
              }

              DLOG(TRACE, "removing cached entry (too old): " << to_remove->first);
              store_.erase (to_remove);

              return 1;
            }
          }

          mutable store_type store_;
          size_type max_size_;
        };
      }
    }
  }
}

#endif
