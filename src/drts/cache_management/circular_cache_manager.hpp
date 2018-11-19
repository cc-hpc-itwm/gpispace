#pragma once

#include <drts/cache_management/cache_manager.hpp>

#include <unordered_map>
#include <map>
#include <unordered_set>

namespace drts
{
  namespace cache
  {

    struct cached_data
    {
      cached_data(unsigned long o, unsigned long s)
      : _offset(o), _size(s)
      {}
      ~cached_data() = default;

      unsigned long offset() const
      {
        return _offset;
      }
      unsigned long size() const
      {
        return _size;
      }

    private:
      unsigned long _offset;
      unsigned long _size;
    };

    class circular_cache_manager : public cache_manager
    {
    public:
      circular_cache_manager(unsigned long);
      ~circular_cache_manager() = default;

      bool is_cached(Dataid const&) const override;
      std::unordered_set<Dataid> add_chunk_list_to_cache(std::unordered_map<Dataid, unsigned long> const&) override;

      unsigned long offset(Dataid const&) const override;
      void clear() override;

    protected:
      unsigned long _free_offset;
      unsigned long _current_position;
      std::unordered_map<Dataid, cached_data> _cache_map;
      std::map<unsigned long, Dataid> _offsets_map;
      unsigned long _gap_begin_offset;
      unsigned long _gap_end_offset;
    };
  }
}
