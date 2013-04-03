// mirko.rahn@itwm.fraunhofer.de

#define FUSE_USE_VERSION 26

#include <fuse.h>

#include <gpifs/state.hpp>
#include <gpifs/file.hpp>
#include <gpifs/segment.hpp>

#include <gpifs/log.hpp>

#include <algorithm>
#include <iterator>

#include <boost/optional.hpp>

#include <cstring> // memset
#include <errno.h>

// ************************************************************************* //

static const gpifs::state::buffer::slot_t num_slots (256);
static const gpifs::state::buffer::size_t size_per_slot (256);

static gpifs::state::state state (num_slots, size_per_slot);

// ************************************************************************* //

extern "C" int
gpifs_getattr (const char * path, struct stat * stbuf)
{
  LOG ("getattr " << path);

  state.refresh();

  const gpifs::state::maybe_splitted_path msp (state.split (path));

  if (!msp)
    {
      return -ENOENT;
    }

  const gpifs::state::splitted_path & sp (*msp);

  int res (0);

  memset (stbuf, 0, sizeof (*stbuf));

  stbuf->st_uid = getuid();
  stbuf->st_gid = getgid();

  if (state.is_directory (sp))
    {
      stbuf->st_mode = S_IFDIR | S_IRWXU;
      stbuf->st_nlink = 2;

      if (sp.handle)
        {
          stbuf->st_size = gpifs::file::num::handle();

          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = state.get_ctime (*sp.handle);
        }
      else
        {
          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = state.time_refresh();

          if (sp.segment_id)
            {
              stbuf->st_size = state.size_segment (*sp.segment_id);
            }
          else if (sp.segment == gpifs::segment::shared())
            {
              stbuf->st_size = state.size_segment_shared ();
            }
          else if (sp.segment == gpifs::segment::proc())
            {
              stbuf->st_size = gpifs::file::num::proc();

              if (!state.error_get())
                {
                  stbuf->st_size -= 1;
                }

              stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
                = std::max (state.time_refresh(), state.error_time());
            }
        }
    }
  else if (state.is_file (sp))
    {
      stbuf->st_nlink = 1;

      if (gpifs::file::is_valid::proc (*sp.file))
        {
          if (sp.file == gpifs::file::name::proc::error())
            {
              if (state.error_get())
                {
                  stbuf->st_size = (*(state.error_get())).size();

                  stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
                    = state.error_time();

                  stbuf->st_mode = S_IFREG | S_IRUSR;
                }
              else
                {
                  res = -ENOENT;
                }
            }
          else
            {
              stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
                = state.time_refresh();

              stbuf->st_mode = S_IFREG | S_IWUSR;
            }
        }
      else if (gpifs::file::is_valid::handle (*sp.file) && sp.handle)
        {
          stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime
            = state.get_ctime (*sp.handle);

          if (sp.file == gpifs::file::name::handle::type())
            {
              stbuf->st_mode = S_IFREG | S_IRUSR;
              stbuf->st_size = state.get_segment_string (*sp.handle).size();
            }
          else if (sp.file == gpifs::file::name::handle::name())
            {
              stbuf->st_mode = S_IFREG | S_IRUSR;
              stbuf->st_size = state.get_name (*sp.handle).size();
            }
          else if (sp.file == gpifs::file::name::handle::data())
            {
              stbuf->st_mode = S_IFREG | S_IRUSR | S_IWUSR;
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

  state.refresh();

  const gpifs::state::maybe_splitted_path msp (state.split (path));

  if (!msp)
    {
      return -ENOENT;
    }

  const gpifs::state::splitted_path & sp (*msp);

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

  const gpifs::state::maybe_splitted_path msp (state.split (path));

  if (!msp)
    {
      return -ENOENT;
    }

  const gpifs::state::splitted_path & sp (*msp);

  int res (0);

  if (state.is_file (sp))
    {
      if (gpifs::file::is_valid::proc (*sp.file))
        {
          if (sp.file == gpifs::file::name::proc::error())
            {
              if ((fi->flags & O_ACCMODE) != O_RDONLY)
                {
                  res = -EACCES;
                }
            }
          else if ((fi->flags & O_ACCMODE) != O_WRONLY)
            {
              res = -EACCES;
            }
          else if (  (sp.file == gpifs::file::name::proc::alloc())
                  || (sp.file == gpifs::file::name::proc::free())
                  )
            {
              if (!state.slot_avail())
                {
                  state.error_set ("no more filehandles available");

                  res = -EAGAIN;
                }
              else
                {
                  fi->fh = state.slot_pop();
                }
            }
        }
      else if (gpifs::file::is_valid::handle (*sp.file))
        {
          if (  (sp.file == gpifs::file::name::handle::type())
             || (sp.file == gpifs::file::name::handle::name())
             )
            {
              if ((fi->flags & O_ACCMODE) != O_RDONLY)
                {
                  res = -EACCES;
                }
            }
          else if (sp.file == gpifs::file::name::handle::data())
            {
              fi->direct_io = 1;
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
      size = std::min (size, static_cast<size_t>(str.size() - offset));

      std::copy (str.begin(), str.begin() + size, buf + offset);
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
  LOG ("read " << path << ", size " << size << ", offset " << offset);

  const gpifs::state::maybe_splitted_path msp (state.split (path));

  if (!msp)
    {
      return -ENOENT;
    }

  const gpifs::state::splitted_path & sp (*msp);

  if (state.is_file (sp))
    {
      if (gpifs::file::is_valid::handle (*sp.file) && sp.handle)
        {
          if (sp.file == gpifs::file::name::handle::type())
            {
              size = copy_string ( state.get_segment_string (*sp.handle)
                                 , buf
                                 , size
                                 , offset
                                 );
            }
          else if (sp.file == gpifs::file::name::handle::name())
            {
              size = copy_string ( state.get_name (*sp.handle)
                                 , buf
                                 , size
                                 , offset
                                 );
            }
          else if (sp.file == gpifs::file::name::handle::data())
            {
              size = state.read (*sp.handle, buf, size, offset);
            }
        }
      else if (gpifs::file::is_valid::proc (*sp.file))
        {
          if (sp.file == gpifs::file::name::proc::error() && state.error_get())
            {
              size = copy_string ( *(state.error_get())
                                 , buf
                                 , size
                                 , offset
                                 );
            }
          else
            {
              size = 0;
            }
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
            , struct fuse_file_info * fi
            )
{
  LOG ("write " << path << ", size " << size << ", offset " << offset);

  const gpifs::state::maybe_splitted_path msp (state.split (path));

  if (!msp)
    {
      return -ENOENT;
    }

  const gpifs::state::splitted_path & sp (*msp);

  if (state.is_directory (sp))
    {
      return -EACCES;
    }
  else if (state.is_file (sp))
    {
      if (gpifs::file::is_valid::proc (*sp.file))
        {
          if (  (sp.file == gpifs::file::name::proc::alloc())
             || (sp.file == gpifs::file::name::proc::free())
             )
            {
              state.slot_write (fi->fh, buf, size, offset);
            }
          else
            {
              return -EACCES;
            }
        }
      else if (gpifs::file::is_valid::handle (*sp.file))
        {
          if (sp.file == gpifs::file::name::handle::data() && sp.handle)
            {
              size = state.write (*sp.handle, buf, size, offset);
            }
          else
            {
              return -EACCES;
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

extern "C" int
gpifs_release (const char * path, struct fuse_file_info * fi)
{
  LOG ("release " << path);

  const gpifs::state::maybe_splitted_path msp (state.split (path));

  if (!msp)
    {
      return -ENOENT;
    }

  const gpifs::state::splitted_path & sp (*msp);

  if (state.is_file (sp))
    {
      if (gpifs::file::is_valid::proc (*sp.file))
        {
          if (sp.file == gpifs::file::name::proc::refresh())
            {
              state.refresh();
            }
          else if (sp.file == gpifs::file::name::proc::alloc())
            {
              char * pos (state.slot_begin (fi->fh));

              gpifs::util::parse::parser<char *> parser
                (pos, state.slot_end (fi->fh));

              const boost::optional<gpifs::alloc::descr> descr
                (gpifs::alloc::parse (parser));

              if (descr)
                {
                  state.alloc (*descr);
                }
              else
                {
                  state.error_set
                    (parser.error_string ("alloc: could not parse descr"));
                }

              state.slot_push (fi->fh);
            }
          else if (sp.file == gpifs::file::name::proc::free())
            {
              char * pos (state.slot_begin (fi->fh));

              gpifs::util::parse::parser<char *> parser
                (pos, state.slot_end (fi->fh));

              const boost::optional<gpifs::alloc::id_t> id
                (gpifs::id::parse (parser));

              if (id)
                {
                  state.free (*id);
                }
              else
                {
                  state.error_set
                    (parser.error_string ("free: could not parse id"));
                }

              state.slot_push (fi->fh);
            }
        }
    }

  return 0;
}

// ************************************************************************* //

extern "C" int
gpifs_utimens (const char * path, const struct timespec *)
{
  LOG ("utimens " << path);

  return 0;
}

extern "C" int
gpifs_truncate (const char * path, off_t offset)
{
  LOG ("truncate " << path << " to " << offset);

  return 0;
}

extern "C" int gpifs_mknod (const char * path, mode_t, dev_t)
{
  LOG ("mknod " << path);

  return -EACCES;
}

extern "C" int gpifs_mkdir (const char * path, mode_t)
{
  LOG ("mkdir " << path);

  return -EACCES;
}

extern "C" int gpifs_unlink (const char * path)
{
  LOG("unlink " << path);

  return -EACCES;
}

extern "C" int gpifs_rmdir (const char * path)
{
  LOG("rmdir " << path);

  return -EACCES;
}

extern "C" int gpifs_symlink (const char * path, const char *)
{
  LOG("symlink " << path);

  return -EACCES;
}

extern "C" int gpifs_rename (const char * path, const char *)
{
  LOG("rename " << path);

  return -EACCES;
}

extern "C" int gpifs_link (const char * path, const char *)
{
  LOG("link " << path);

  return -EACCES;
}

extern "C" int gpifs_chmod (const char * path, mode_t)
{
  LOG("chmod " << path);

  return -EACCES;
}

extern "C" int gpifs_chown (const char * path, uid_t, gid_t)
{
  LOG("chown " << path);

  return -EACCES;
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
  oper.mknod = gpifs_mknod;
  oper.mkdir = gpifs_mkdir;
  oper.unlink = gpifs_unlink;
  oper.rmdir = gpifs_rmdir;
  oper.symlink = gpifs_symlink;
  oper.rename = gpifs_rename;
  oper.link = gpifs_link;
  oper.chmod = gpifs_chmod;
  oper.chown = gpifs_chown;

  int ret (0);

  ret = state.init();

  if (ret != 0)
  {
    if (state.error_get())
    {
      std::cerr << "could not initialize: " << *state.error_get() << std::endl;
    }
    return ret;
  }

  ret = fuse_main (argc, argv, &oper, NULL);
  if (ret != 0)
  {
    if (state.error_get())
    {
      std::cerr << "fuse_main returned non-zero: " << *state.error_get() << std::endl;
    }
  }

  ret = state.finalize();
  if (ret != 0)
  {
    if (state.error_get())
    {
      std::cerr << "fuse_main returned non-zero: " << *state.error_get() << std::endl;
    }
  }

  return ret;
}
