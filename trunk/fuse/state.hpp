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

namespace gpi_fuse
{
  namespace state
  {
    struct state
    {
    public:
      state (const comm::comm & comm)
        : _segment_map (0)
        , _alloc_map (0)
        , _comm (comm)
      {
        build_maps ();
      }

      bool is_file (const std::string & path)
      {
        bool ret (false);

        if (path != "/")
          {
            const splitted_path sp (split (util::strip_prefix ("/", path)));

            ret = sp.file;
          }

        return ret;
      }

      bool is_directory (const std::string & path)
      {
        bool ret (false);

        if (path == "/")
          {
            ret = true;
          }
        else
          {
            const splitted_path sp (split (util::strip_prefix ("/", path)));

            ret = !sp.file && (sp.segment || sp.handle);
          }

        return ret;
      }

      void readdir ( const std::string & path
                   , void * buf
                   , fuse_fill_dir_t filler
                   )
      {
        _readdir (util::strip_prefix ("/", path), buf, filler);
      }

    private:
      typedef boost::unordered_map<segment::id_t, alloc::set_t> segment_map_t;
      typedef boost::unordered_map<alloc::id_t,segment::id_t> alloc_map_t;

      segment_map_t _segment_map;
      alloc_map_t _alloc_map;
      comm::comm _comm;

      void build_maps ()
      {
        segment::id_list_t segments; _comm.list_segments (&segments);

        for ( segment::id_list_t::const_iterator segment (segments.begin())
            ; segment != segments.end()
            ; ++segment
            )
          {
            _segment_map[*segment] = alloc::set_t ();

            alloc::id_list_t allocs; _comm.list_allocs (*segment, &allocs);

            for ( alloc::id_list_t::const_iterator alloc (allocs.begin())
                ; alloc != allocs.end()
                ; ++alloc
                )
              {
                _segment_map[*segment].insert (*alloc);
                _alloc_map[*alloc] = *segment;
              }
          }
      }

      // ******************************************************************* //

#ifdef COMM_TEST
    public:
#endif
      splitted_path split (const std::string & path)
      {
        splitted_path sp;

        if (util::starts_with (segment::proc(), path))
          {
            sp.segment = segment::proc();

            split_proc (util::strip_prefix (segment::proc(), path), sp);
          }
        else if (util::starts_with (segment::global(), path))
          {
            sp.segment = segment::global();

            split_handle (util::strip_prefix (segment::global(), path), sp, 0);
          }
        else if (util::starts_with (segment::local(), path))
          {
            sp.segment = segment::local();

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

#ifdef COMM_TEST
    private:
#endif
      bool separator ( std::string::const_iterator & pos
                     , splitted_path & sp
                     )
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
                      )
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
                               )
      {
        std::string::const_iterator pos (path.begin());

        if (pos != path.end())
          {
            sp.handle = id::parse (pos, path.end());

            if (_alloc_map.find (*sp.handle) == _alloc_map.end())
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
                        )
      {
        std::string::const_iterator pos (path.begin());

        if (pos != path.end() && separator (pos, sp))
          {
            sp.handle = id::parse (pos, path.end());

            if (  _segment_map[segment].find (*sp.handle)
               == _segment_map[segment].end()
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
                        )
      {
        std::string::const_iterator pos (path.begin());

        if (pos != path.end() && separator (pos, sp))
          {
            sp.segment_id = id::parse (pos, path.end());

            if (_segment_map.find (*sp.segment_id) != _segment_map.end())
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
                             , const segment::id_t & s
                             , void * buf
                             , fuse_fill_dir_t filler
                             )
      {
        if (sp.handle)
          {
            fill (file::data(), buf, filler);
            fill (file::type(), buf, filler);
          }
        else
          {
            for ( alloc::set_t::const_iterator alloc (_segment_map[s].begin())
                ; alloc != _segment_map[s].end()
                ; ++alloc
                )
              {
                fill ( boost::lexical_cast<std::string>(*alloc)
                     , buf
                     , filler
                     );
              }
          }
      }

      void _readdir ( const std::string & path
                    , void * buf
                    , fuse_fill_dir_t filler
                    )
      {
        if (path.empty())
          {
            fill (segment::global(), buf, filler);
            fill (segment::local(), buf, filler);
            fill (segment::shared(), buf, filler);
            fill (segment::proc(), buf, filler);

            for ( alloc_map_t::const_iterator alloc (_alloc_map.begin())
                ; alloc != _alloc_map.end()
                ; ++alloc
                )
              {
                fill ( boost::lexical_cast<std::string>(alloc->first)
                     , buf
                     , filler
                     );
              }
          }
        else
          {
            const splitted_path sp (split (path));

            if (!sp.segment)
              {
                if (sp.handle)
                  {
                    fill (file::data(), buf, filler);
                    fill (file::type(), buf, filler);
                  }
                else
                  {
                    throw
                      std::runtime_error ("STRANGE! FuseInvariants broken!?");
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
                for ( segment_map_t::const_iterator segment
                        (_segment_map.begin())
                    ; segment != _segment_map.end()
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
      }
    };
  } // namespace state
} // namespace gpi_fuse

#endif
