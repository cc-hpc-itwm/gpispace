// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_STATE_HPP
#define FUSE_STATE_HPP 1

#define FUSE_USE_VERSION 26

#include <fuse.h>

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <boost/optional.hpp>

#include <string>
#include <stack>

#include <util.hpp>
#include <file.hpp>
#include <id.hpp>
#include <segment.hpp>
#include <alloc.hpp>
#include <comm.hpp>
#include <splitted_path.hpp>
#include <comm.hpp>

#include <ctime>

namespace gpifs
{
  namespace state
  {
    namespace buffer
    {
      typedef unsigned int slot_t;
      typedef std::size_t size_t;

      struct state : public boost::noncopyable
      {
        char * buf;
        const slot_t num_slots;
        const size_t size_per_slot;
        std::stack<slot_t> slots;

        state ()
          : buf (NULL)
          , num_slots (0)
          , size_per_slot (0)
          , slots ()
        {}

        state ( const slot_t & _num_slots
              , const size_t & _size_per_slot
              )
          : buf (new char[_num_slots * _size_per_slot])
          , num_slots (_num_slots)
          , size_per_slot (_size_per_slot)
          , slots ()
        {
          for (buffer::slot_t slot (0); slot < num_slots; ++slot)
            {
              slots.push (slot);
            }
        }

        ~state ()
        {
          if (buf != NULL)
            {
              delete[] buf;
            }
        }
      };
    } // namespace buffer

    // ********************************************************************* //

    struct state : public boost::noncopyable
    {
    public:
      typedef boost::unordered_map<segment::id_t, alloc::id_set_t> segments_t;
      typedef boost::unordered_map<alloc::id_t,alloc::alloc> allocs_t;
      typedef boost::optional<std::string> error_t;

      state ()
        : _segments (0)
        , _allocs (0)
        , _comm ()
        , _time_refresh (0)
        , _buffer ()
        , _error ()
        , _time_error (0)
      {}

      state ( const buffer::slot_t & num_slots
            , const buffer::size_t & size_per_slot
            )
        : _segments (0)
        , _allocs (0)
        , _comm ()
        , _time_refresh (0)
        , _buffer (num_slots, size_per_slot)
        , _error ()
        , _time_error (0)
      {}

      // ******************************************************************* //

      int init ()
      {
        int res (0);

        error_clear();

        res = _comm.init ();

        if (res < 0)
          {
            error_set ("failed to initialize connection to gpi manager");
          }
        else
          {
            res = refresh();

            if (res < 0)
              {
                error_set ("failed to get listings from gpi manager");
              }
          }

        return res;
      }
      int finalize ()
      {
        int res (0);

        error_clear ();

        res = _comm.finalize ();

        if (res < 0)
          {
            error_set ("failed to finalize connection to gpi manager");
          }

        return res;
      }

      // ******************************************************************* //

      char * slot_begin (const buffer::slot_t & slot)
      {
        return _buffer.buf + slot * _buffer.size_per_slot;
      }
      char * slot_end (const buffer::slot_t & slot)
      {
        return _buffer.buf + (slot + 1) * _buffer.size_per_slot;
      }
      bool slot_avail () const
      {
        return !_buffer.slots.empty();
      }
      buffer::slot_t slot_pop ()
      {
        const buffer::slot_t slot (_buffer.slots.top());

        _buffer.slots.pop();

        std::fill (slot_begin (slot), slot_end (slot), 0);

        return slot;
      }
      void slot_push (const buffer::slot_t & slot)
      {
        _buffer.slots.push (slot);
      }
      buffer::size_t slot_write ( const buffer::slot_t & slot
                                , const char * buf
                                , buffer::size_t size
                                , const buffer::size_t & offset
                                )
      {
        if (offset < _buffer.size_per_slot)
          {
            size = std::min (size, _buffer.size_per_slot - offset);

            std::copy (buf, buf + size, slot_begin (slot) + offset);
          }
        else
          {
            size = 0;
          }

        return size;
      }

      // ******************************************************************* //

      int refresh ()
      {
        int res (0);

        error_clear ();

        segment::id_list_t segments;

        res = _comm.list_segments (&segments);

        if (res < 0)
          {
            error_set ("failed to get segment list from gpi manager");
          }

        for ( segment::id_list_t::const_iterator segment (segments.begin())
            ; segment != segments.end() && res == 0
            ; ++segment
            )
          {
            _segments[*segment] = alloc::id_set_t ();

            alloc::list_t allocs;

            res = _comm.list_allocs (*segment, &allocs);

            if (res < 0)
              {
                std::ostringstream err;

                err << "failed to get alloc list for segment " << *segment;

                error_set (err.str());
              }

            for ( alloc::list_t::const_iterator alloc (allocs.begin())
                    ; alloc != allocs.end()
                    ; ++alloc
                )
              {
                _segments[*segment].insert (alloc->id());
                _allocs[alloc->id()] = *alloc;
              }
          }

        _time_refresh = time (NULL);

        return res;
      }

      const time_t & time_refresh () const
      {
        return _time_refresh;
      }

      // ******************************************************************* //

      void error_set (const std::string & err)
      {
        _error = err;

        _time_error = time (NULL);
      }
      void error_clear ()
      {
        _error = error_t (boost::none);
      }
      const error_t & error_get () const
      {
        return _error;
      }
      const time_t & error_time () const
      {
        return _time_error;
      }

      // ******************************************************************* //

      std::size_t read ( const alloc::id_t & id
                       , char * buf
                       , size_t size
                       , const off_t offset
                       )
      {
        LOG ( "state: read from handle " << id << ":" << offset
            << " " << size << " bytes"
            );

        return 0;
      }
      std::size_t write ( const alloc::id_t & id
                        , const char * buf
                        , size_t size
                        , const off_t offset
                        )
      {
        LOG ( "state: write to handle " << id << ":" << offset
            << " " << size << " bytes"
            );

        return 0;
      }

      // ******************************************************************* //

      int alloc (const alloc::descr & descr)
      {
        int res (_comm.alloc (descr));

        if (res < 0)
          {
            std::ostringstream err;

            err << "failed to allocate " << descr;

            error_set (err.str());
          }
        else
          {
            res = refresh();
          }

        return res;
      }
      int free (const alloc::id_t & id)
      {
        int res (_comm.free (id));

        if (res < 0)
          {
            std::ostringstream err;

            err << "failed to free the allocation with the id " << id;

            error_set (err.str());
          }
        else
          {
            const allocs_t::const_iterator alloc (_allocs.find (id));

            if (alloc != _allocs.end())
              {
                segments_t::iterator segment
                  (_segments.find (alloc->second.segment()));

                if (segment != _segments.end())
                  {
                    segment->second.erase (id);
                  }

                _allocs.erase (alloc);
              }
          }

        return res;
      }

      // ******************************************************************* //

      std::string get_segment_string (const alloc::id_t & id) const
      {
        const allocs_t::const_iterator alloc (_allocs.find (id));

        return (alloc == _allocs.end())
          ? "SEGMENT_FOR_UNKNOWN_ALLOCATION"
          : segment::string (alloc->second.segment());
      }
      std::string get_name (const alloc::id_t & id) const
      {
        const allocs_t::const_iterator alloc (_allocs.find (id));

        return (alloc == _allocs.end())
          ? "NAME_OF_UNKNOWN_ALLOCATION"
          : alloc->second.name();
      }
      alloc::size_t get_size (const alloc::id_t & id) const
      {
        const allocs_t::const_iterator alloc (_allocs.find (id));

        return (alloc == _allocs.end())
          ? 0
          : alloc->second.size();
      }
      time_t get_ctime (const alloc::id_t & id) const
      {
        const allocs_t::const_iterator alloc (_allocs.find (id));

        return (alloc == _allocs.end())
          ? 0
          : alloc->second.ctime();
      }

      // ******************************************************************* //

      std::size_t size_segment (const segment::id_t & segment) const
      {
        const segments_t::const_iterator seg (_segments.find (segment));

        return (seg == _segments.end()) ? 0 : seg->second.size();
      }
      std::size_t size_segment_shared () const
      {
        return _segments.size() - 2;
      }

      // ******************************************************************* //

      bool is_file (const splitted_path & sp) const
      {
        return sp.file;
      }
      bool is_directory (const splitted_path & sp) const
      {
        return !sp.file && (sp.segment || sp.handle);
      }

      // ******************************************************************* //

      void readdir ( const splitted_path & sp
                   , void * buf
                   , fuse_fill_dir_t filler
                   ) const
      {
        if (!sp.segment)
          {
            if (sp.handle)
              {
                fill (file::name::handle::data(), buf, filler);
                fill (file::name::handle::type(), buf, filler);
                fill (file::name::handle::name(), buf, filler);
              }
            else
              {
                throw
                  std::runtime_error ("STRANGE! FuseInvariants broken!?");
              }
          }
        else if (sp.segment == segment::root())
          {
            fill (segment::global(), buf, filler);
            fill (segment::local(), buf, filler);
            fill (segment::shared(), buf, filler);
            fill (segment::proc(), buf, filler);

            for ( allocs_t::const_iterator alloc (_allocs.begin())
                ; alloc != _allocs.end()
                ; ++alloc
                )
              {
                fill ( boost::lexical_cast<std::string>(alloc->first)
                     , buf
                     , filler
                     );
              }
          }
        else if (sp.segment == segment::proc())
          {
            fill (file::name::proc::alloc(), buf, filler);
            fill (file::name::proc::free(), buf, filler);
            fill (file::name::proc::refresh(), buf, filler);

            if (_error)
              {
                fill (file::name::proc::error(), buf, filler);
              }
          }
        else if (sp.segment == segment::global())
          {
            fill_from_segment (sp, 0, buf, filler);
          }
        else if (sp.segment == segment::local())
          {
            fill_from_segment (sp, 1, buf, filler);
          }
        else if (sp.segment == segment::shared())
          {
            for ( segments_t::const_iterator segment (_segments.begin())
                ; segment != _segments.end()
                ; ++segment
                )
              {
                if (segment->first > 1)
                  {
                    fill
                      ( boost::lexical_cast<std::string>(segment->first)
                      , buf
                      , filler
                      );
                  }
              }
          }
        else
          {
            fill_from_segment (sp, *sp.segment_id, buf, filler);
          }
      }

      // ******************************************************************* //

      splitted_path split (const std::string & path) const
      {
        if (util::starts_with (segment::root(), path))
          {
            return split (util::strip_prefix (segment::root(), path));
          }
        else
          {
            splitted_path sp;

            if (path.empty())
              {
                sp.segment = segment::root();
              }
            if (util::starts_with (segment::proc(), path))
              {
                sp.segment = segment::proc();

                split_proc (util::strip_prefix (segment::proc(), path), sp);
              }
            else if (util::starts_with (segment::global(), path))
              {
                sp.segment = segment::global();
                sp.segment_id = 0;

                split_handle
                  (util::strip_prefix (segment::global(), path), sp, 0);
              }
            else if (util::starts_with (segment::local(), path))
              {
                sp.segment = segment::local();
                sp.segment_id = 1;

                split_handle
                  (util::strip_prefix (segment::local(), path), sp, 1);
              }
            else if (util::starts_with (segment::shared(), path))
              {
                sp.segment = segment::shared();

                split_shared
                  (util::strip_prefix (segment::shared(), path), sp);
              }
            else
              {
                split_handle_direct (path, sp);
              }

            return sp;
          }
      }

      // ******************************************************************* //

    private:
      segments_t _segments;
      allocs_t _allocs;
      comm::comm _comm;
      time_t _time_refresh;
      buffer::state _buffer;
      error_t _error;
      time_t _time_error;

      // ******************************************************************* //

      bool separator ( std::string::const_iterator & pos
                     , splitted_path & sp
                     ) const
      {
        bool ret (true);

        if (*pos != '/')
          {
            sp.clear();

            ret = false;
          }

        ++pos;

        return ret;
      }
      void split_proc ( const std::string & path
                      , splitted_path & sp
                      ) const
      {
        std::string::const_iterator pos (path.begin());

        if (pos != path.end() && separator (pos, sp))
          {
            sp.file = std::string (pos, path.end());

            if (!file::is_valid::proc (*sp.file))
              {
                sp.clear();
              }
          }
      }
      void split_handle_direct ( const std::string & path
                               , splitted_path & sp
                               ) const
      {
        std::string::const_iterator pos (path.begin());

        if (pos != path.end())
          {
            util::parse::parser_string parse_state (pos, path.end());

            sp.handle = id::parse (parse_state);

            if (!sp.handle || (_allocs.find (*sp.handle) == _allocs.end()))
              {
                sp.clear();
              }
            else if (pos != path.end() && separator (pos, sp))
              {
                sp.file = std::string (pos, path.end());

                if (!file::is_valid::handle (*sp.file))
                  {
                    sp.clear();
                  }
              }
          }
      }
      void split_handle ( const std::string & path
                        , splitted_path & sp
                        , const segment::id_t segment
                        ) const
      {
        std::string::const_iterator pos (path.begin());

        if (pos != path.end() && separator (pos, sp))
          {
            util::parse::parser_string parse_state (pos, path.end());

            sp.handle = id::parse (parse_state);

            const segments_t::const_iterator seg (_segments.find (segment));

            if (  (seg == _segments.end())
               || (  sp.handle
                  && seg->second.find (*sp.handle) == seg->second.end()
                  )
               )
              {
                sp.clear();
              }
            else if (pos != path.end() && separator (pos, sp))
              {
                sp.file = std::string (pos, path.end());

                if (!file::is_valid::handle (*sp.file))
                  {
                    sp.clear();
                  }
              }
          }
      }
      void split_shared ( const std::string & path
                        , splitted_path & sp
                        ) const
      {
        std::string::const_iterator pos (path.begin());

        if (pos != path.end() && separator (pos, sp))
          {
            util::parse::parser_string parse_state (pos, path.end());

            sp.segment_id = id::parse (parse_state);

            if (sp.segment_id)
              {
                if (_segments.find (*sp.segment_id) != _segments.end())
                  {
                    sp.segment = segment::shared()
                      + "/"
                      + boost::lexical_cast<std::string> (*sp.segment_id)
                      ;
                  }
                else
                  {
                    sp.clear();
                  }

                split_handle ( std::string (pos, path.end())
                             , sp
                             , *sp.segment_id
                             );
              }
            else
              {
                split_handle_direct ( std::string (pos, path.end())
                                    , sp
                                    );
              }
          }
      }

      // ******************************************************************* //

      void fill ( const std::string & path
                , void * buf
                , fuse_fill_dir_t filler
                ) const
      {
        filler (buf, path.c_str(), NULL, 0);
      }
      void fill_from_segment ( const splitted_path & sp
                             , const segment::id_t & segment
                             , void * buf
                             , fuse_fill_dir_t filler
                             ) const
      {
        if (sp.handle)
          {
            fill (file::name::handle::data(), buf, filler);
            fill (file::name::handle::type(), buf, filler);
            fill (file::name::handle::name(), buf, filler);
          }
        else
          {
            const segments_t::const_iterator seg (_segments.find(segment));

            if (seg != _segments.end())
              {
                for ( alloc::id_set_t::const_iterator id (seg->second.begin())
                    ; id != seg->second.end()
                    ; ++id
                    )
                  {
                    fill ( boost::lexical_cast<std::string>(*id)
                         , buf
                         , filler
                         );
                  }
              }
          }
      }
    };
  } // namespace state
} // namespace gpifs

#endif
