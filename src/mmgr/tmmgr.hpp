#pragma once

#include <map>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  namespace vmem
  {
    using Word_t = unsigned long;
    using Size_t = unsigned long;

    using Offset_t = Size_t;
    using Handle_t = Size_t;
    using MemSize_t = Word_t;
    using Align_t = Word_t;
    using Count_t = Word_t;

    namespace error
    {
#define MEMBER(_type, _name)                             \
      public: _type _name() const { return _ ## _name; } \
      private: _type _ ## _name

      class unknown_handle : public std::invalid_argument
      {
        MEMBER (Handle_t, handle);

     public:
        unknown_handle (Handle_t handle)
          : std::invalid_argument ("Unknown Handle " + std::to_string (handle))
          , _handle (handle)
        {}
      };
#undef MEMBER
    }

    class tmmgr
    {
    public:
      tmmgr (MemSize_t, Align_t);

      enum ResizeReturn_t
      { RESIZE_SUCCESS,
        RESIZE_BELOW_HIGHWATER,
        RESIZE_BELOW_MEMUSED,
        RESIZE_FAILURE
      };

      ResizeReturn_t resize (MemSize_t);

      enum AllocReturn_t
        { ALLOC_SUCCESS,
          ALLOC_DUPLICATE_HANDLE,
          ALLOC_INSUFFICIENT_CONTIGUOUS_MEMORY,
          ALLOC_INSUFFICIENT_MEMORY,
          ALLOC_FAILURE
        };

      AllocReturn_t alloc (Handle_t, MemSize_t);

      void free (Handle_t);
      std::pair<Offset_t, MemSize_t> offset_size (Handle_t) const;

      MemSize_t memsize() const
      {
        return _mem_size;
      }
      MemSize_t memfree() const
      {
        return _mem_free;
      }
      MemSize_t highwater() const
      {
        return _high_water;
      }
      Count_t numhandle() const
      {
        return _handle_to_offset_and_size.size();
      }

    private:
      std::unordered_map<Handle_t, std::pair<Offset_t, MemSize_t>>
        _handle_to_offset_and_size;
      std::unordered_map<Offset_t, Handle_t> _offset_to_handle;
      std::map<MemSize_t, std::unordered_multiset<Offset_t>>
        _free_offset_by_size;
      std::unordered_map<Offset_t, Size_t> _free_segment_start;
      std::unordered_map<Offset_t, Size_t> _free_segment_end;
      Align_t _align;
      MemSize_t _mem_size;
      MemSize_t _mem_free;
      MemSize_t _high_water;

      void delete_free_segment (Offset_t, MemSize_t);
      void insert_free_segment (Offset_t, MemSize_t);

      void insert_handle (Handle_t, Offset_t, MemSize_t);
      void delete_handle (Handle_t, std::pair<Offset_t, MemSize_t>);
    };
  }
}
