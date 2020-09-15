#pragma once

#include <boost/format.hpp>

#include <map>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace iml_client
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

      class invalid_argument : public std::invalid_argument
      {
      public:
        invalid_argument (boost::format const& f)
          : std::invalid_argument (f.str())
        {}
      };
      class runtime_error : public std::runtime_error
      {
      public:
        runtime_error (boost::format const& f)
          : std::runtime_error (f.str())
        {}
      };

      class unknown_handle : public invalid_argument
      {
        MEMBER (Handle_t, handle);

     public:
        unknown_handle (Handle_t handle)
          : invalid_argument ( boost::format ("Unknown Handle '%1%'")
                             % handle
                             )
          , _handle (handle)
        {}
      };

      namespace alloc
      {
        class duplicate_handle : public invalid_argument
        {
          MEMBER (Handle_t, handle);

        public:
          duplicate_handle (Handle_t handle)
            : invalid_argument ( boost::format ("Duplicate Handle '%1%'")
                               % handle
                               )
          {}
        };

        class insufficient_contiguous_memory : public runtime_error
        {
          MEMBER (Handle_t, handle);
          MEMBER (MemSize_t, size_unaligned);
          MEMBER (MemSize_t, size_aligned);
          MEMBER (MemSize_t, mem_free);

        public:
          insufficient_contiguous_memory ( Handle_t handle
                                         , MemSize_t size_unaligned
                                         , MemSize_t size_aligned
                                         , MemSize_t mem_free
                                         )
            : runtime_error
              ( boost::format ("Insufficient contiguous memory:"
                              " Trying to alloc '%2%' (aligned: '%3%') bytes"
                              " for handle '%1%' and mem_free '%4%',"
                              " but no free block of memory is large enough"
                              )
              % handle
              % size_unaligned
              % size_aligned
              % mem_free
              )
            , _handle (handle)
            , _size_unaligned (size_unaligned)
            , _size_aligned (size_aligned)
            , _mem_free (mem_free)
          {}
        };

        class insufficient_memory : public runtime_error
        {
          MEMBER (Handle_t, handle);
          MEMBER (MemSize_t, size_unaligned);
          MEMBER (MemSize_t, size_aligned);
          MEMBER (MemSize_t, mem_free);

        public:
          insufficient_memory ( Handle_t handle
                              , MemSize_t size_unaligned
                              , MemSize_t size_aligned
                              , MemSize_t mem_free
                              )
            : runtime_error
              ( boost::format ("Insufficient memory:"
                              " Trying to alloc '%2%' (aligned: '%3%') bytes"
                              " for handle '%1%', but mem_free is '%4%'"
                              )
              % handle
              % size_unaligned
              % size_aligned
              % mem_free
              )
            , _handle (handle)
            , _size_unaligned (size_unaligned)
            , _size_aligned (size_aligned)
            , _mem_free (mem_free)
          {}
        };
      }

      namespace resize
      {
        class below_mem_used : public runtime_error
        {
          MEMBER (MemSize_t, new_size_unaligned);
          MEMBER (MemSize_t, new_size_aligned);
          MEMBER (MemSize_t, mem_size);
          MEMBER (MemSize_t, mem_free);

        public:
          below_mem_used ( MemSize_t new_size_unaligned
                         , MemSize_t new_size_aligned
                         , MemSize_t mem_size
                         , MemSize_t mem_free
                         )
            : runtime_error
              ( boost::format ("Resize below memory used:"
                              " Trying to resize to '%1%' (aligned: '%2%') bytes"
                              " but mem_size is '%3%' and mem_free '%4%'"
                              )
              % new_size_unaligned
              % new_size_aligned
              % mem_size
              % mem_free
              )
            , _new_size_unaligned (new_size_unaligned)
            , _new_size_aligned (new_size_aligned)
            , _mem_size (mem_size)
            , _mem_free (mem_free)
          {}
        };

        class below_high_water : public runtime_error
        {
          MEMBER (MemSize_t, new_size_unaligned);
          MEMBER (MemSize_t, new_size_aligned);
          MEMBER (MemSize_t, high_water);

        public:
          below_high_water ( MemSize_t new_size_unaligned
                           , MemSize_t new_size_aligned
                           , MemSize_t high_water
                           )
            : runtime_error
              ( boost::format ("Resize below high water:"
                              " Trying to resize to '%1%' (aligned: '%2%') bytes"
                              " but high_water is '%3%'"
                              )
              % new_size_unaligned
              % new_size_aligned
              % high_water
              )
            , _new_size_unaligned (new_size_unaligned)
            , _new_size_aligned (new_size_aligned)
            , _high_water (high_water)
          {}
        };
      }
#undef MEMBER
    }

    class tmmgr
    {
    public:
      tmmgr (MemSize_t, Align_t);

      void resize (MemSize_t);
      std::pair<Offset_t, MemSize_t> alloc (Handle_t, MemSize_t);
      void free (Handle_t);

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
    };
  }
}
