
#include <drts/cache_management/circular_cache_manager.hpp>
#include <drts/cache_management/error.hpp>

#include <boost/format.hpp>

namespace drts
{
  namespace cache
  {
    circular_cache_manager::circular_cache_manager(unsigned long total_size)
    : cache_manager(total_size)
    , _free_offset(0)
    , _current_position(0)
    , _cache_map()
    , _offsets_map()
    , _gap_begin_offset(0)
    , _gap_end_offset(0)
    {}

    std::unordered_set<Dataid> circular_cache_manager::add_chunk_list_to_cache(std::unordered_map<Dataid, unsigned long> const& dataids)
    {
      std::unordered_set<Dataid> already_cached_dataids;

      for (auto dataid_it : dataids)
      {
        auto dataid = dataid_it.first;
        auto data_size = dataid_it.second;

        if (_cache_map.count(dataid) > 0)
        {
          already_cached_dataids.emplace(dataid);
          continue;
        }

        if (_free_offset + data_size <= this->size())
        {
          _cache_map.emplace(dataid, cached_data(_free_offset, data_size));
          _offsets_map.emplace(_free_offset, dataid);
          _free_offset += data_size;
          _current_position = _free_offset;
        }
        else
        {
          if (_offsets_map.size() <= 0)
          {
            throw drts::cache::error::buffer_larger_than_cache_size(data_size, this->size());
          }

          if (_gap_begin_offset + data_size > this->size())
          {
            _gap_begin_offset = _current_position;
            _gap_end_offset = _current_position;
          }

          int pass = 0;
          bool ready = false;
          do {
            auto it = _offsets_map.find(_gap_end_offset);
            while (it != _offsets_map.end() && _gap_begin_offset + data_size > _gap_end_offset)
            {
              if (dataids.count(it->second) <= 0)
              { // can delete the dataid from cache if it does not belong to the current list of chunks
                _gap_end_offset = it->first + _cache_map.at(it->second).size();

                _cache_map.erase(it->second);
                it = _offsets_map.erase(it);

              }
              else
              {
                _gap_begin_offset = it->first + _cache_map.at(it->second).size();
                it++;
              }
            }

            if (it == _offsets_map.end()) // deleted everything until the end of the cache buffer
            {
              _gap_end_offset = this->size();
            }

            if (data_size <= _gap_end_offset - _gap_begin_offset) // data fits in the gap
            {
              _cache_map.emplace(dataid, cached_data(_gap_begin_offset, data_size));
              _offsets_map.emplace(_gap_begin_offset, dataid);
              _gap_begin_offset += data_size;
              _current_position = _gap_begin_offset;
              ready = true;
            }
            else
            {
              // restart search from the beginning
              _gap_begin_offset = 0;
              _gap_end_offset = 0;
              if (_offsets_map.size() > 0)
              {
                _gap_end_offset = _offsets_map.begin()->first;
              }
            }

            ++pass;
          } while(pass <= 2 && !ready);

          if (!ready)
          {
            throw drts::cache::error::insufficient_space_available_in_cache
            (this->size(), _free_offset, _gap_begin_offset, _gap_end_offset);
          }

          if ((_gap_begin_offset >= _free_offset) || (_gap_end_offset >= _free_offset))
          {
            _free_offset = (_gap_begin_offset >= _free_offset)?_gap_begin_offset: _free_offset;
            _gap_begin_offset = _current_position;
            _gap_end_offset = _current_position;
          }

        }
      }
      return already_cached_dataids;
    }


    unsigned long circular_cache_manager::offset(Dataid const& dataid) const
    {
      if (!is_cached(dataid))
      {
        throw drts::cache::error::buffer_not_in_cache(dataid);
      }
      return _cache_map.at(dataid).offset();
    }

    bool circular_cache_manager::is_cached(Dataid const& dataid) const
     {
       return (_cache_map.count(dataid) > 0);
     }
  }
}

