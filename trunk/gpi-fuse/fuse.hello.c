#define FUSE_USE_VERSION  26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static const char *big_path = "/big";
static char *big_str = NULL;
static const off_t big_size = (1 << 20);

static FILE * log = NULL;

static uid_t uid;
static gid_t gid;

#define LOG(FMT,ARGS...) \
  fprintf(log, "[%s:%d] " FMT, __FILE__, __LINE__, ##ARGS); fflush (log)

static int
hello_getattr (const char *path, struct stat *stbuf)
{
  LOG ("getattr: path %s\n", path);

  int res = 0;
  memset (stbuf, 0, sizeof (struct stat));

  stbuf->st_uid = uid;
  stbuf->st_gid = gid;

  if (strcmp (path, "/") == 0)
    {
      stbuf->st_mode = S_IFDIR | 0700;
      stbuf->st_nlink = 2;
    }
  else if (strcmp (path, hello_path) == 0)
    {
      stbuf->st_mode = S_IFREG | 0600;
      stbuf->st_nlink = 1;
      stbuf->st_size = strlen (hello_str);
    }
  else if (strcmp (path, big_path) == 0)
    {
      stbuf->st_mode = S_IFREG | 0600;
      stbuf->st_nlink = 1;
      stbuf->st_size = big_size;
    }
  else
    {
      res = -ENOENT;
    }

  return res;
}

static int
hello_readdir (const char *path, void *buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi)
{
  LOG ("readdir: path %s\n", path);

  (void) offset;
  (void) fi;

  if (strcmp (path, "/") != 0)
    {
      return -ENOENT;
    }

  filler (buf, ".", NULL, 0);
  filler (buf, "..", NULL, 0);
  filler (buf, hello_path + 1, NULL, 0);
  filler (buf, big_path + 1, NULL, 0);

  return 0;
}

static int
hello_open (const char *path, struct fuse_file_info *fi)
{
  LOG ("open: path %s\n", path);

  if (  (strcmp (path, hello_path) != 0)
     && (strcmp (path, big_path) != 0)
     )
    {
      return -ENOENT;
    }

  fi->direct_io = 1;

/*   if ((fi->flags & 3) != O_RDONLY) */
/*     { */
/*       return -EACCES; */
/*     } */

  return 0;
}

static int
hello_read (const char *path, char *buf, size_t size, off_t offset,
            struct fuse_file_info *fi)
{
  LOG ("read: path %s size %lu offset %lu\n", path, size, offset);

  size_t len;
  (void) fi;

  if (strcmp (path, "/") == 0)
    {
      size = 0;
    }
  else if (strcmp (path, hello_path) == 0)
    {
      len = strlen (hello_str);
      if (offset < len)
        {
          if (offset + size > len)
            {
              size = len - offset;
            }
          memcpy (buf, hello_str + offset, size);
        }
    }
  else if (strcmp (path, big_path) == 0)
    {
      len = big_size;
      if (offset < len)
        {
          if (offset + size > len)
            {
              size = len - offset;
            }
          memcpy (buf, big_str + offset, size);
        }
    }
  else
    {
      return -ENOENT;
    }

  return size;
}

static struct fuse_operations hello_oper = {
  .getattr = hello_getattr,
  .readdir = hello_readdir,
  .open = hello_open,
  .read = hello_read,
};

int
main (int argc, char *argv[])
{
  uid = getuid();
  gid = getgid();

  big_str = malloc (big_size);

  if (!big_str)
    {
      fprintf (stderr, "could not allocate memory\n");
      exit(EXIT_FAILURE);
    }

  {
    off_t i;
    for (i = 0; i < big_size; ++i)
      {
        big_str[i] = 65 + i % 26;
      }
  }

  log = fopen ("fuse.hello.log", "w");

  if (!log)
    {
      fprintf (stderr, "could not open log file\n");
      exit(EXIT_FAILURE);
    }

  LOG ("fuse_main: start...\n");

  const int ret = fuse_main (argc, argv, &hello_oper, NULL);

  LOG ("fuse_main: ...done\n");

  free (big_str);

  return ret;
}
