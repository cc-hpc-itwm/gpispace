// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/vmem/tmmgr.hpp>

#include <boost/preprocessor/stringize.hpp>

#include <fmt/core.h>
#include <stdexcept>
#include <string>
#include <utility>

namespace iml_client
{
  namespace vmem
  {
    namespace
    {
      constexpr MemSize_t alignDown (MemSize_t Size, Align_t Align)
      {
        return (Size & ~(Align - 1));
      }

      constexpr Offset_t alignUp (MemSize_t Size, Align_t Align)
      {
        return alignDown (Size + Align - 1, Align);
      }

#define precondition(cond)                                              \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      throw std::logic_error                                            \
        { fmt::format ( "[tmmgr:{}] assertion '{}' failed."             \
                      , __LINE__                                        \
                      , BOOST_PP_STRINGIZE (cond)                       \
                      )                                                 \
        };                                                              \
    }                                                                   \
  } while (false)

    }

    tmmgr::tmmgr (MemSize_t MemSize, Align_t Align)
      : _align (Align)
      , _mem_size (alignDown (MemSize, _align))
      , _mem_free (_mem_size)
    {
      insert_free_segment (0, _mem_size);
    }

    void tmmgr::resize (MemSize_t NewSizeUnaligned)
    {
      MemSize_t const NewSize (alignDown (NewSizeUnaligned, _align));

      if (NewSize < _mem_size - _mem_free)
      {
        throw error::resize::below_mem_used
          (NewSizeUnaligned, NewSize, _mem_size, _mem_free);
      }

      if (NewSize < _high_water)
      {
        throw error::resize::below_high_water
          (NewSizeUnaligned, NewSize, _high_water);
      }

      MemSize_t const OldSize (_mem_size);

      if (NewSize == OldSize)
      {
        return;
      }

      MemSize_t const Delta
        ((NewSize > OldSize) ? (NewSize - OldSize) : (OldSize - NewSize));

      auto free_segment_end (_free_segment_end.find (OldSize));

      if (free_segment_end != _free_segment_end.end())
      {
        /* longer a free segment */

        MemSize_t const FreeSize (free_segment_end->second);

        precondition (NewSize > OldSize || Delta <= FreeSize);

        MemSize_t const NewFreeSize =
          ((NewSize > OldSize) ? (FreeSize + Delta) : (FreeSize - Delta));

        Offset_t const OffsetStart (OldSize - FreeSize);

        delete_free_segment (OffsetStart, FreeSize);
        insert_free_segment (OffsetStart, NewFreeSize);
      }
      else
      {
        /* add a new free segment */

        precondition (NewSize > OldSize);

        insert_free_segment (OldSize, Delta);
      }

      _mem_size = NewSize;
      _mem_free =
        ((NewSize > OldSize) ? (_mem_free + Delta) : (_mem_free - Delta));
    }

    std::pair<Offset_t, MemSize_t> tmmgr::alloc
      (Handle_t Handle, MemSize_t SizeUnaligned)
    {
      MemSize_t const Size (alignUp (SizeUnaligned, _align));

      if (_mem_free < Size)
      {
        throw error::alloc::insufficient_memory
          (Handle, SizeUnaligned, Size, _mem_free);
      }

      if (_handle_to_offset_and_size.count (Handle) > 0)
      {
        throw error::alloc::duplicate_handle (Handle);
      }

      auto pos (_free_offset_by_size.lower_bound (Size));

      if (pos == _free_offset_by_size.end())
      {
        throw error::alloc::insufficient_contiguous_memory
          (Handle, SizeUnaligned, Size, _mem_free);
      }

      Offset_t const Offset (*pos->second.begin());

      _mem_free -= Size;

      if (_high_water < Offset + Size)
      {
        _high_water = Offset + Size;
      }

      auto PSizeStart (_free_segment_start.find (Offset));

      precondition (PSizeStart != _free_segment_start.end());
      precondition (Size <= PSizeStart->second);
      precondition ( _free_segment_end.find (Offset + PSizeStart->second)
                   != _free_segment_end.end()
                   );
      precondition ( PSizeStart->second
                   == _free_segment_end.at (Offset + PSizeStart->second)
                   );

      Size_t const OldFreeSize (PSizeStart->second);
      Size_t const NewFreeSize (PSizeStart->second - Size);

      delete_free_segment (Offset, OldFreeSize);
      insert_free_segment (Offset + Size, NewFreeSize);

      _handle_to_offset_and_size.emplace (Handle, std::make_pair (Offset, Size));
      _offset_to_handle.emplace (Offset, Handle);

      return {Offset, Size};
    }

    void tmmgr::free (Handle_t Handle)
    {
      auto pos (_handle_to_offset_and_size.find (Handle));

      if (pos == _handle_to_offset_and_size.end())
      {
        throw error::unknown_handle (Handle);
      }

      Offset_t const Offset (pos->second.first);
      MemSize_t const Size (pos->second.second);

      _mem_free += Size;

      _handle_to_offset_and_size.erase (Handle);
      _offset_to_handle.erase (Offset);

      auto PSizeL (_free_segment_end.find (Offset));
      auto PSizeR (_free_segment_start.find (Offset + Size));

      if (Offset + Size == _high_water)
      {
        _high_water = Offset - ((PSizeL == _free_segment_end.end()) ? 0 : PSizeL->second);
      }

      if (  PSizeL != _free_segment_end.end()
         && PSizeR != _free_segment_start.end()
         )
      {
        /* join left and right segment */

        Size_t const SizeL (PSizeL->second);
        Size_t const SizeR (PSizeR->second);

        precondition (Offset >= SizeL);

        Offset_t const OffsetL (Offset - SizeL);
        Offset_t const OffsetR (Offset + Size);

        delete_free_segment (OffsetL, SizeL);
        delete_free_segment (OffsetR, SizeR);
        insert_free_segment (OffsetL, SizeL + Size + SizeR);
      }
      else if (PSizeL != _free_segment_end.end())
      {
        /* longer the left segment */

        MemSize_t const SizeL (PSizeL->second);

        precondition (Offset >= SizeL);

        Offset_t const OffsetL (Offset - SizeL);

        delete_free_segment (OffsetL, SizeL);
        insert_free_segment (OffsetL, SizeL + Size);
      }
      else if (PSizeR != _free_segment_start.end())
      {
        /* longer the right segment */

        MemSize_t const SizeR (PSizeR->second);

        Offset_t const OffsetR (Offset + Size);

        delete_free_segment (OffsetR, SizeR);
        insert_free_segment (Offset, Size + SizeR);
      }
      else
      {
        insert_free_segment (Offset, Size);
      }
    }

    void tmmgr::delete_free_segment (Offset_t OffsetStart, MemSize_t Size)
    {
      Offset_t const OffsetEnd (OffsetStart + Size);

      {
        auto pos_size (_free_offset_by_size.find (Size));
        if (pos_size == _free_offset_by_size.end())
        {
          throw std::runtime_error ("Missing Size " + std::to_string (Size));
        }
        auto& offsets (pos_size->second);
        auto pos_offset (offsets.find (OffsetStart));
        if (pos_offset == offsets.end())
        {
          throw std::logic_error ("Missing Offset " + std::to_string (OffsetStart));
        }
        offsets.erase (pos_offset);
        if (offsets.empty())
        {
          _free_offset_by_size.erase (pos_size);
        }
      }
      _free_segment_start.erase (OffsetStart);
      _free_segment_end.erase (OffsetEnd);
    }

    void tmmgr::insert_free_segment (Offset_t OffsetStart, MemSize_t Size)
    {
      if (Size > 0)
      {
        Offset_t const OffsetEnd (OffsetStart + Size);

        _free_offset_by_size[Size].emplace (OffsetStart);
        _free_segment_start.emplace (OffsetStart, Size);
        _free_segment_end.emplace (OffsetEnd, Size);
      }
    }
  }
}
