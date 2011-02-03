// mirko.rahn@itwm.fraunhofer.de

#define COMM_TEST 1

#define FUSE_USE_VERSION 26

#include <fuse.h>

#include <state.hpp>
#include <file.hpp>
#include <segment.hpp>

#include <algorithm>

#include <cstring> // memset
#include <errno.h> // ENOENT

// ************************************************************************* //

static gpi_fuse::comm::comm comm;

// ************************************************************************* //

extern "C" int
gpi_getattr ( const char * path
            , struct stat * stbuf
            )
{
  gpi_fuse::state::state state (comm);
  gpi_fuse::state::splitted_path sp (state.split (path));

  int res (0);

  memset (stbuf, 0, sizeof (*stbuf));

  stbuf->st_uid = getuid();
  stbuf->st_gid = getgid();

  if (state.is_directory (sp))
    {
      stbuf->st_mode = S_IFDIR | 0700;
      stbuf->st_nlink = 2;

      if (sp.handle)
        {
          stbuf->st_size = gpi_fuse::file::num_valid_handle_file();

          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = state.get_ctime (*sp.handle);
        }
      else
        {
          if (sp.segment_id)
            {
              stbuf->st_size = state.size_segment (*sp.segment_id);
            }
          else if (sp.segment == gpi_fuse::segment::shared())
            {
              stbuf->st_size = state.size_segment_shared ();
            }
          else if (sp.segment == gpi_fuse::segment::proc())
            {
              stbuf->st_size = gpi_fuse::file::num_valid_proc_file();
            }

          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = comm.time_start();
        }
    }
  else if (state.is_file (sp))
    {
      stbuf->st_mode = S_IFREG | 0600;
      stbuf->st_nlink = 1;

      if (sp.file == gpi_fuse::file::type() && sp.handle)
        {
          stbuf->st_size = state.get_segment_string (*sp.handle).size();
        }
      else if (sp.file == gpi_fuse::file::name() && sp.handle)
        {
          stbuf->st_size = state.get_name (*sp.handle).size();
        }
      else if (sp.file == gpi_fuse::file::data() && sp.handle)
        {
          stbuf->st_size = state.get_size (*sp.handle);
        }

      if (sp.handle)
        {
          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = state.get_ctime (*sp.handle);
        }
      else
        {
          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = comm.time_start();
        }
    }
  else
    {
      res = -ENOENT;
    }

  return res;
}

// ************************************************************************* //

extern "C" int
gpi_readdir ( const char * path
            , void * buf
            , fuse_fill_dir_t filler
            , off_t offset
            , struct fuse_file_info * fi
            )
{
  gpi_fuse::state::state state (comm);
  gpi_fuse::state::splitted_path sp (state.split (path));

  int res (0);

  (void) offset;
  (void) fi;

  if (!state.is_directory (sp))
    {
      res = -ENOENT;
    }
  else
    {
      filler (buf, ".", NULL, 0);
      filler (buf, "..", NULL, 0);

      state.readdir (sp, buf, filler);
    }

  return res;
}

// ************************************************************************* //

extern "C" int
gpi_open (const char * path, struct fuse_file_info * fi)
{
  gpi_fuse::state::state state (comm);
  gpi_fuse::state::splitted_path sp (state.split (path));

  int res (0);

  if (state.is_file (sp))
    {
      fi->direct_io = 1;
    }
  else
    {
      res = -ENOENT;
    }

  return res;
}

// ************************************************************************* //

static size_t copy_string ( const std::string & str
                          , char * buf
                          , size_t size
                          , const off_t offset
                          )
{
  if (offset < static_cast<off_t> (str.size()))
    {
      size = std::min (size, str.size() - offset);

      std::copy ( str.begin() + offset
                , str.begin() + offset + size
                , buf
                );
    }
  else
    {
      size = 0;
    }

  return size;
}

extern "C" int
gpi_read ( const char *path
         , char *buf
         , size_t size
         , off_t offset
         , struct fuse_file_info *fi
         )
{
  gpi_fuse::state::state state (comm);
  gpi_fuse::state::splitted_path sp (state.split (path));

  if (state.is_file (sp))
    {
      if (sp.file == gpi_fuse::file::type() && sp.handle)
        {
          size = copy_string ( state.get_segment_string (*sp.handle)
                             , buf
                             , size
                             , offset
                             );
        }
      else if (sp.file == gpi_fuse::file::name() && sp.handle)
        {
          size = copy_string ( state.get_name (*sp.handle)
                             , buf
                             , size
                             , offset
                             );
        }
      else if (sp.file == gpi_fuse::file::data() && sp.handle)
        {
          // do the read here, receive data in buf
          size = 0;
        }
      else
        {
          size = 0;
        }
    }
  else if (state.is_directory (sp))
    {
      size = 0;
    }
  else
    {
      return -ENOENT;
    }

  return size;
}

// ************************************************************************* //

extern "C" int
gpi_write ( const char * path
          , const char * buf
          , size_t size
          , off_t offset
          , struct fuse_file_info *
          )
{
  gpi_fuse::state::state state (comm);
  gpi_fuse::state::splitted_path sp (state.split (path));

  if (state.is_file (sp))
    {
      // do the write here, send data from buf
      // *sp.handle!
      size = 0;
    }
  else if (state.is_directory (sp))
    {
      size = 0;
    }
  else
    {
      return -ENOENT;
    }

  return size;
}

// ************************************************************************* //

int
main (int argc, char **argv)
{
  static struct fuse_operations oper;

  memset (&oper, 0, sizeof (oper));

  oper.getattr = gpi_getattr;
  oper.readdir = gpi_readdir;
  oper.open = gpi_open;
  oper.read = gpi_read;
  oper.write = gpi_write;

  comm.init();

  const int ret (fuse_main (argc, argv, &oper, NULL));

  comm.finalize();

  return ret;
}
