// mirko.rahn@itwm.fraunhofer.de

#define COMM_TEST 1

#define FUSE_USE_VERSION 26

#include <fuse.h>

#include <state.hpp>
#include <file.hpp>
#include <segment.hpp>

#include <log.h>

#include <algorithm>
#include <iterator>

#include <cstring> // memset
#include <errno.h> // ENOENT

// ************************************************************************* //

static gpi_fuse::state::state state;

// ************************************************************************* //

extern "C" int
gpifs_getattr ( const char * path
              , struct stat * stbuf
              )
{
  LOG ("getattr " << path);

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
          stbuf->st_size = gpi_fuse::file::num::handle();

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
              stbuf->st_size = gpi_fuse::file::num::proc();
            }

          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = state.time_refresh();
        }
    }
  else if (state.is_file (sp))
    {
      stbuf->st_nlink = 1;

      if (gpi_fuse::file::is_valid::proc (*sp.file))
        {
          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = state.time_refresh();

          stbuf->st_mode = S_IFREG | 0200;
        }
      else if (gpi_fuse::file::is_valid::handle (*sp.file))
        {
          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = state.get_ctime (*sp.handle);

          if (sp.file == gpi_fuse::file::type() && sp.handle)
            {
              stbuf->st_mode = S_IFREG | 0400;
              stbuf->st_size = state.get_segment_string (*sp.handle).size();
            }
          else if (sp.file == gpi_fuse::file::name() && sp.handle)
            {
              stbuf->st_mode = S_IFREG | 0400;
              stbuf->st_size = state.get_name (*sp.handle).size();
            }
          else if (sp.file == gpi_fuse::file::data() && sp.handle)
            {
              stbuf->st_mode = S_IFREG | 0600;
              stbuf->st_size = state.get_size (*sp.handle);
            }
        }
      else
        {
          res = -ENOENT;
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
gpifs_readdir ( const char * path
              , void * buf
              , fuse_fill_dir_t filler
              , off_t
              , struct fuse_file_info *
              )
{
  LOG ("readdir " << path);

  gpi_fuse::state::splitted_path sp (state.split (path));

  int res (0);

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
gpifs_open (const char * path, struct fuse_file_info * fi)
{
  LOG ("open " << path << ", flags " << (fi->flags & O_ACCMODE));

  gpi_fuse::state::splitted_path sp (state.split (path));

  int res (0);

  if (state.is_file (sp))
    {
      fi->direct_io = 1;

      if (gpi_fuse::file::is_valid::proc (*sp.file))
        {
          if ((fi->flags & O_ACCMODE) != O_WRONLY)
            {
              res = -EACCES;
            }
        }
      else if (gpi_fuse::file::is_valid::handle (*sp.file))
        {
          if (sp.file == gpi_fuse::file::type())
            {
              if ((fi->flags & O_ACCMODE) != O_RDONLY)
                {
                  res = -EACCES;
                }
            }
          else if (sp.file == gpi_fuse::file::name())
            {
              if ((fi->flags & O_ACCMODE) != O_RDONLY)
                {
                  res = -EACCES;
                }
            }
          else if (sp.file == gpi_fuse::file::data())
            {
            }
          else
            {
              res = -ENOENT;
            }
        }
      else
        {
          res = -ENOENT;
        }
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
gpifs_read ( const char *path
           , char *buf
           , size_t size
           , off_t offset
           , struct fuse_file_info *
           )
{
  LOG ("read " << path << ", size " << size << ", offset " << offset)

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
gpifs_write ( const char * path
            , const char * buf
            , size_t size
            , off_t offset
            , struct fuse_file_info *
            )
{
  LOG ("write " << path << ", size " << size << ", offset " << offset);

  gpi_fuse::state::splitted_path sp (state.split (path));

  if (state.is_directory (sp))
    {
      return -EACCES;
    }
  else if (state.is_file (sp))
    {
      if (gpi_fuse::file::is_valid::proc (*sp.file))
        {
          if (sp.file == gpi_fuse::file::alloc())
            {
              LOG ("ALLOC");
            }
          else if (sp.file == gpi_fuse::file::free())
            {
              gpi_fuse::alloc::id_t id (gpi_fuse::id::parse (buf, buf + size));

              LOG ("FREE " << id);

              state.free (id);
            }
        }
      else if (gpi_fuse::file::is_valid::handle (*sp.file))
        {
          if (sp.file == gpi_fuse::file::data() && sp.handle)
            {
              LOG ("write " << size << " bytes "
                  << " into " << path << ":" << offset
                  );

              // do the write here, send data from buf
              // *sp.handle!
            }
          else
            {
              return -ENOENT;
            }
        }
    }
  else
    {
      return -ENOENT;
    }

  return size;
}

// ************************************************************************* //

extern "C" int gpifs_release (const char * path, struct fuse_file_info *)
{
  LOG ("release " << path);

  gpi_fuse::state::splitted_path sp (state.split (path));

  if (state.is_file (sp))
    {
      if (gpi_fuse::file::is_valid::proc (*sp.file))
        {
          if (sp.file == gpi_fuse::file::refresh())
            {
              LOG ("REFRESH");

              state.refresh();
            }
        }
    }

  return 0;
}

// ************************************************************************* //

extern "C" int gpifs_utimens (const char * path, const struct timespec *)
{
  LOG ("utimens " << path);

  return 0;
}

// ************************************************************************* //

extern "C" int gpifs_truncate (const char * path, off_t offset)
{
  LOG ("truncate " << path << " to " << offset);

  return 0;
}

// ************************************************************************* //

int
main (int argc, char **argv)
{
  static struct fuse_operations oper;

  memset (&oper, 0, sizeof (oper));

  oper.getattr = gpifs_getattr;
  oper.readdir = gpifs_readdir;
  oper.open = gpifs_open;
  oper.read = gpifs_read;
  oper.write = gpifs_write;
  oper.release = gpifs_release;
  oper.utimens = gpifs_utimens;
  oper.truncate = gpifs_truncate;

  state.init();

  const int ret (fuse_main (argc, argv, &oper, NULL));

  state.finalize();

  return ret;
}
