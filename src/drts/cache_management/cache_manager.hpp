#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace drts
{
  namespace cache
  {
    using Dataid = std::string;

    class cache_manager
    {
    public:
      cache_manager(unsigned long);
      virtual ~cache_manager() = default;
      unsigned long size() const;

      virtual bool is_cached(Dataid const&) const;
      virtual std::unordered_set<Dataid> add_chunk_list_to_cache(std::unordered_map<Dataid, unsigned long> const&);

      virtual unsigned long offset(Dataid const&) const;
      virtual void clear();

    private:
      unsigned long _total_size;

      //unsigned long _free_offset;
      //std::unordered_map<data_id_type, std::string> _cache_map;

    };
  }
}
