#include <drts/cache_management/cache_manager.hpp>

#include <stdexcept>

namespace drts
{
  namespace cache
  {
      cache_manager::cache_manager(unsigned long s)
        : _total_size(s)
      {}

      unsigned long cache_manager::size() const
      {
        return _total_size;
      }

      unsigned long cache_manager::offset(Dataid const&) const
      {
        throw std::runtime_error("cache_manager::offset not implemented");
      }

      std::unordered_set<Dataid> cache_manager::add_chunk_list_to_cache(std::unordered_map<Dataid, unsigned long> const&)
      {
        throw std::runtime_error("cache_manager::add_chunk_list_to_cache not implemented");
      }

      bool cache_manager::is_cached(Dataid const&) const
      {
        throw std::runtime_error("cache_manager::is_cached not implemented");
      }

      void cache_manager::clear()
      {
        throw std::runtime_error("cache_manager::clear not implemented");
      }
  }
}
