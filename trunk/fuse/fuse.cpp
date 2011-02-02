// mirko.rahn@itwm.fraunhofer.de

#define FUSE_USE_VERSION 26

#include <fuse.h>

#include <string>
#include <sstream>
#include <fstream>
#include <list>

#include <stdexcept>

#define COMM_TEST 1

static std::ofstream LOG;

#include <util.hpp>
#include <id.hpp>
#include <segment.hpp>
#include <alloc.hpp>
#include <comm.hpp>
#include <state.hpp>
#include <splitted_path.hpp>

// #include <boost/date_time/posix_time/posix_time_types.hpp>

#include <cstring> // memset
#include <errno.h> // ENOENT

// ************************************************************************* //

static gpi_fuse::comm::comm comm;

extern "C" int
gpi_getattr ( const char * path
            , struct stat * stbuf
            )
{
  int res (0);

  memset (stbuf, 0, sizeof (*stbuf));

  stbuf->st_uid = getuid();
  stbuf->st_gid = getgid();

  gpi_fuse::state::state state (comm);

  if (state.is_directory (path))
    {
      stbuf->st_mode = S_IFDIR | 0700;
      stbuf->st_nlink = 2;
    }
  else if (state.is_file (path))
    {
      stbuf->st_mode = S_IFREG | 0600;
      stbuf->st_nlink = 1;
    }
  else
    {
      res = -ENOENT;
    }

  return res;
}

extern "C" int
gpi_readdir ( const char * path
            , void * buf
            , fuse_fill_dir_t filler
            , off_t offset
            , struct fuse_file_info * fi
            )
{
  int res (0);

  (void) offset;
  (void) fi;

  gpi_fuse::state::state state (comm);

  if (!state.is_directory (path))
    {
      res = -ENOENT;
    }
  else
    {
      filler (buf, ".", NULL, 0);
      filler (buf, "..", NULL, 0);

      state.readdir (path, buf, filler);
    }

  return res;
}


#include <iostream>
int
main (int argc, char **argv)
{
  LOG.open ("gpi_fuse.log");

  if (!LOG)
    {
      throw std::runtime_error ("could not open log file");
    }

  comm.init();

  static struct fuse_operations oper;

  memset (&oper, 0, sizeof (oper));

  oper.getattr = gpi_getattr;
  oper.readdir = gpi_readdir;

  const int ret (fuse_main (argc, argv, &oper, NULL));

  comm.finalize();

  LOG.close();

  return ret;
}
