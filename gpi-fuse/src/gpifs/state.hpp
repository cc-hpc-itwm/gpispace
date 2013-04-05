// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_STATE_HPP
#define FUSE_STATE_HPP 1

#define FUSE_USE_VERSION 26

#include <string.h>

#include <fuse.h>

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <boost/optional.hpp>

#include <string>
#include <stack>

#include <gpifs/util.hpp>
#include <gpifs/file.hpp>
#include <gpifs/id.hpp>
#include <gpifs/segment.hpp>
#include <gpifs/alloc.hpp>
#include <gpifs/comm.hpp>
#include <gpifs/splitted_path.hpp>
#include <gpifs/comm.hpp>

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
        , _error_split ()
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
        , _error_split ()
      {}

      // ******************************************************************* //

      int init ()
      {
        error_clear();

        int res (0);

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
        error_clear ();

        const int res (_comm.finalize ());

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
        //        error_clear ();

        return refresh (boost::none);
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

      const error_t & error_split_get () const
      {
        return _error_split;
      }

      // ******************************************************************* //

      std::size_t read ( const alloc::id_t & id
                       , char * buf
                       , size_t size
                       , const off_t offset
                       )
      {
        return _comm.read (id, buf, size, offset);
      }
      std::size_t write ( const alloc::id_t & id
                        , const char * buf
                        , size_t size
                        , const off_t offset
                        )
      {
        return _comm.write (id, buf, size, offset);
      }

      // ******************************************************************* //

      int alloc (const alloc::descr & descr)
      {
        error_clear();

        int res (_comm.alloc (descr));

        if (res < 0)
          {
            std::ostringstream err;

            err << "failed to allocate "
                << descr
                << ": "
                << strerror (-res)
                << std::endl;
              ;

            error_set (err.str());
          }
        else
          {
            res = refresh (descr.segment());
          }

        return res;
      }
      int free (const alloc::id_t & id)
      {
        error_clear();

        const int res (_comm.free (id));

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
                for ( file::valid_set_t::const_iterator
                        file (file::set::handle().begin())
                    ; file != file::set::handle().end()
                    ; ++file
                    )
                  {
                    fill (*file, buf, filler);
                  }
              }
            else
              {
                throw
                  std::runtime_error ("STRANGE! FuseInvariants broken!?");
              }
          }
        else if (sp.segment == segment::root())
          {
            //            fill (segment::global(), buf, filler);
            //            fill (segment::local(), buf, filler);
            //            fill (segment::shared(), buf, filler);
            fill (segment::proc(), buf, filler);

            for ( allocs_t::const_iterator alloc (_allocs.begin())
                ; alloc != _allocs.end()
                ; ++alloc
                )
              {
                fill ( ::gpifs::id::toHex (alloc->first)
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
            fill_from_segment (sp, segment::GLOBAL, buf, filler);
          }
        else if (sp.segment == segment::local())
          {
            fill_from_segment (sp, segment::LOCAL, buf, filler);
          }
        else if (sp.segment == segment::shared())
          {
            for ( segments_t::const_iterator segment (_segments.begin())
                ; segment != _segments.end()
                ; ++segment
                )
              {
                if (segment->first > segment::LOCAL)
                  {
                    fill
                      ( boost::lexical_cast<std::string>(segment->first)
                      , buf
                      , filler
                      );
                  }
              }
          }
        else if (sp.segment_id)
          {
            fill_from_segment (sp, *sp.segment_id, buf, filler);
          }
      }

      // ******************************************************************* //

      maybe_splitted_path split (const std::string & path)
      {
        _error_split = error_t (boost::none);

        std::string::const_iterator begin (path.begin());

        util::parse::parser<std::string::const_iterator>
          parser (begin, path.end());

        if (!util::parse::require ("/", parser))
          {
            return nothing (parser);
          }
        else
          {
            if (parser.end())
              {
                return maybe_splitted_path (splitted_path (segment::root()));
              }
            else
              {
                switch (tolower (*parser))
                  {
                  case 'p': ++parser;

                    return util::parse::require ("roc", parser)
                      ? split_proc (parser)
                      : nothing (parser)
                      ;

                  case 'g': ++parser;

                    return util::parse::require ("lobal", parser)
                      ? split_handle (parser, segment::GLOBAL)
                      : nothing (parser)
                      ;

                  case 'l': ++parser;

                    return util::parse::require ("ocal", parser)
                      ? split_handle (parser, segment::LOCAL)
                      : nothing (parser)
                      ;

                  case 's': ++parser;

                    return util::parse::require ("hared", parser)
                      ? split_shared (parser)
                      : nothing (parser)
                      ;

                  default:
                    return split_handle (parser, boost::none);
                  }
              }
          }

        return boost::none;
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
      error_t _error_split;

      // ******************************************************************* //

      int refresh (const boost::optional<segment::id_t> & filter)
      {
        segment::id_list_t segments;

        int res (_comm.list_segments (&segments));

        if (res < 0)
          {
            error_set ("failed to get segment list from gpi manager");
          }
        else
          {
            for ( segment::id_list_t::const_iterator segment (segments.begin())
                ; segment != segments.end() && res == 0
                ; ++segment
                )
              {
                if (!filter || *filter == *segment)
                  {
                    res = refresh_segment (*segment);
                  }
              }

            _time_refresh = time (NULL);
          }

        return res;
      }
      int refresh_segment (const segment::id_t & segment)
      {
        _segments[segment] = alloc::id_set_t ();

        alloc::list_t allocs;

        const int res (_comm.list_allocs (segment, &allocs));

        if (res < 0)
          {
            std::ostringstream err;

            err << "failed to get alloc list for segment "
                << segment::string (segment)
              ;

            error_set (err.str());
          }
        else
          {
            for ( alloc::list_t::const_iterator alloc (allocs.begin())
                ; alloc != allocs.end()
                ; ++alloc
                )
              {
                _segments[segment].insert (alloc->id());
                _allocs[alloc->id()] = *alloc;
              }
          }

        return res;
      }

      // ******************************************************************* //

      template<typename IT>
      maybe_splitted_path nothing (util::parse::parser<IT> parser)
      {
        _error_split = parser.error_string ("error splitting path");

        return maybe_splitted_path (boost::none);
      }
      template<typename IT>
      maybe_splitted_path split_handle
      ( util::parse::parser<IT> & parser
      , const boost::optional<segment::id_t> & segment
      )
      {
        const boost::optional<alloc::id_t> handle (id::parse (parser));

        if (!handle)
          {
            return nothing (parser);
          }
        else if (_allocs.find (*handle) == _allocs.end())
          {
            std::ostringstream err;

            err << "allocation id " << *handle << " unknown";

            parser.error_set (err.str());

            return nothing (parser);
          }
        else if (segment && _segments.find (*segment) == _segments.end())
          {
            parser.error_set ("STRANGE! segment unknown!?");

            return nothing (parser);
          }
        else if (  segment
                && (  _segments.find (*segment)->second.find (*handle)
                   == _segments.find (*segment)->second.end()
                   )
                )
          {
            std::ostringstream err;

            err << "allocation id " << *handle
                << " not in segment " << segment::string (*segment)
              ;

            parser.error_set (err.str());

            return nothing (parser);
          }
        else if (parser.end())
          {
            return segment
              ? maybe_splitted_path (splitted_path ( segment::string (*segment)
                                                   , *segment
                                                   , *handle
                                                   )
                                    )
              : maybe_splitted_path (splitted_path (*handle))
              ;
          }
        else if (!util::parse::require ("/", parser))
          {
            return nothing (parser);
          }
        else
          {
            const std::string file (parser.rest());

            if (file::is_valid::handle (file))
              {
                return segment
                  ? maybe_splitted_path
                    ( splitted_path ( segment::string (*segment)
                                    , *segment
                                    , *handle
                                    , file
                                    )
                    )
                  : maybe_splitted_path (splitted_path (*handle, file))
                  ;
              }
            else
              {
                parser.error_set ( "expected one of "
                                 + file::set::string (file::set::handle())
                                 );

                return nothing (parser);
              }
          }

        return nothing (parser);
      }
      template<typename IT>
      maybe_splitted_path split_handle ( util::parse::parser<IT> & parser
                                       , const segment::id_t & segment
                                       )
      {
        const segments_t::const_iterator seg (_segments.find (segment));

        if (seg == _segments.end())
          {
            std::ostringstream err;

            err << "unknown segment " << segment::string (segment);

            parser.error_set (err.str());

            return nothing (parser);
          }
        else if (parser.end())
          {
            return maybe_splitted_path
              (splitted_path ( segment::string (segment)
                             , segment
                             )
              );
          }
        else if (!util::parse::require ("/", parser))
          {
            return nothing (parser);
          }
        else
          {
            return split_handle ( parser
                                , boost::optional<segment::id_t> (segment)
                                );
          }
      }
      template<typename IT>
      maybe_splitted_path split_shared (util::parse::parser<IT> & parser)
      {
        if (parser.end())
          {
            return maybe_splitted_path (splitted_path (segment::shared()));
          }
        else if (!util::parse::require ("/", parser))
          {
            return nothing (parser);
          }
        else
          {
            const boost::optional<segment::id_t> segment (id::parse (parser));

            if (segment)
              {
                if (*segment < 2)
                  {
                    std::ostringstream err;

                    err << "segment_id " << *segment
                        << " is not an id of a shared segment";

                    parser.error_set (err.str());

                    return nothing (parser);
                  }
                else
                  {
                    return split_handle (parser, *segment);
                  }
              }
            else
              {
                return nothing (parser);
              }
          }
      }
      template<typename IT>
      maybe_splitted_path split_proc (util::parse::parser<IT> & parser)
      {
        if (parser.end())
          {
            return maybe_splitted_path (splitted_path (segment::proc()));
          }
        else if (!util::parse::require ("/", parser))
          {
            return nothing (parser);
          }
        else
          {
            const std::string file (parser.rest());

            if (file::is_valid::proc (file))
              {
                return maybe_splitted_path (splitted_path ( segment::proc()
                                                          , file
                                                          )
                                           );
              }
            else
              {
                parser.error_set ( "expected one of "
                                 + file::set::string (file::set::proc())
                                 );

                return nothing (parser);
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
            for ( file::valid_set_t::const_iterator
                    file (file::set::handle().begin())
                ; file != file::set::handle().end()
                ; ++file
                )
              {
                fill (*file, buf, filler);
              }
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
                    fill (::gpifs::id::toHex (*id), buf, filler);
                  }
              }
          }
      }
    };
  } // namespace state
} // namespace gpifs

#endif
