// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_STATE_HPP
#define FUSE_STATE_HPP 1

#define FUSE_USE_VERSION 26

#include <fuse.h>

#include <boost/unordered_map.hpp>

#include <string>

#include <util.hpp>
#include <file.hpp>
#include <id.hpp>
#include <segment.hpp>
#include <alloc.hpp>
#include <comm.hpp>
#include <splitted_path.hpp>

#include <ctime>

namespace gpi_fuse
{
  namespace state
  {
    struct state
    {
    public:
      typedef boost::unordered_map<segment::id_t, alloc::id_set_t> segments_t;
      typedef boost::unordered_map<alloc::id_t,alloc::descr_t> allocs_t;

      state (const comm::comm & comm)
        : _segments (0)
        , _allocs (0)
        , _comm (comm)
      {
        build_maps ();
      }

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

      std::size_t size_segment (const segment::id_t & segment) const
      {
        const segments_t::const_iterator seg (_segments.find (segment));

        return (seg == _segments.end()) ? 0 : seg->second.size();
      }
      std::size_t size_segment_shared () const
      {
        return _segments.size() - 2;
      }

      bool is_file (const splitted_path & sp) const
      {
        return sp.file;
      }
      bool is_directory (const splitted_path & sp) const
      {
        return !sp.file && (sp.segment || sp.handle);
      }

      void readdir ( const splitted_path & sp
                   , void * buf
                   , fuse_fill_dir_t filler
                   ) const
      {
        if (!sp.segment)
          {
            if (sp.handle)
              {
                fill (file::data(), buf, filler);
                fill (file::type(), buf, filler);
                fill (file::name(), buf, filler);
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
            fill (file::alloc(), buf, filler);
            fill (file::free(), buf, filler);
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

                split_handle (util::strip_prefix (segment::global(), path), sp, 0);
              }
            else if (util::starts_with (segment::local(), path))
              {
                sp.segment = segment::local();
                sp.segment_id = 1;

                split_handle (util::strip_prefix (segment::local(), path), sp, 1);
              }
            else if (util::starts_with (segment::shared(), path))
              {
                sp.segment = segment::shared();

                split_shared ( util::strip_prefix (segment::shared(), path), sp);
              }
            else
              {
                split_handle_direct (path, sp);
              }

            return sp;
          }
      }

    private:
      segments_t _segments;
      allocs_t _allocs;
      comm::comm _comm;

      void build_maps ()
      {
        segment::id_list_t segments; _comm.list_segments (&segments);

        for ( segment::id_list_t::const_iterator segment (segments.begin())
            ; segment != segments.end()
            ; ++segment
            )
          {
            _segments[*segment] = alloc::id_set_t ();

            alloc::list_t allocs; _comm.list_allocs (*segment, &allocs);

            for ( alloc::list_t::const_iterator alloc (allocs.begin())
                ; alloc != allocs.end()
                ; ++alloc
                )
              {
                _segments[*segment].insert (alloc->id());
                _allocs[alloc->id()] = *alloc;
              }
          }
      }

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

            if (!file::is_valid_proc_file (*sp.file))
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
            sp.handle = id::parse (pos, path.end());

            if (_allocs.find (*sp.handle) == _allocs.end())
              {
                sp.clear();
              }
            else if (pos != path.end() && separator (pos, sp))
              {
                sp.file = std::string (pos, path.end());

                if (!file::is_valid_handle_file (*sp.file))
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
            sp.handle = id::parse (pos, path.end());

            const segments_t::const_iterator seg (_segments.find(segment));

            if (  seg == _segments.end()
               || seg->second.find (*sp.handle) == seg->second.end()
               )
              {
                sp.clear();
              }
            else if (pos != path.end() && separator (pos, sp))
              {
                sp.file = std::string (pos, path.end());

                if (!file::is_valid_handle_file (*sp.file))
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
            sp.segment_id = id::parse (pos, path.end());

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

            if (pos != path.end())
              {
                split_handle ( std::string (pos, path.end())
                             , sp
                             , *sp.segment_id
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
            fill (file::data(), buf, filler);
            fill (file::type(), buf, filler);
            fill (file::name(), buf, filler);
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
} // namespace gpi_fuse

#endif
