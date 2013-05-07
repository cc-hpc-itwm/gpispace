
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define FAILURE(fmt,...)                       \
  do                                           \
    {                                          \
      printf ( "FAILURE: [%u]: " fmt ": %s\n"  \
             , __LINE__                        \
             , __VA_ARGS__                     \
             , strerror (errno)                \
             );                                \
                                               \
      exit (EXIT_FAILURE);                     \
    }                                          \
  while (0)

void run ( const char * filename
         , const off_t pos
         , const size_t count
         )
{
  int fd = open (filename, O_RDONLY);

  if (fd < 0)
    {
      FAILURE ("cannot open file %s for reading", filename);
    }

  struct stat sbuf;

  if (fstat(fd, &sbuf) < 0)
    {
      FAILURE ("cannot stat %i (%s)", fd, filename);
    }

  printf ("dev %lu\n", sbuf.st_dev);
  printf ("ino %lu\n", sbuf.st_ino);
  printf ("mode %u\n", sbuf.st_mode);
  printf ("nlink %lu\n", sbuf.st_nlink);
  printf ("uid %u\n", sbuf.st_uid);
  printf ("gid %u\n", sbuf.st_gid);
  printf ("rdev %lu\n", sbuf.st_rdev);
  printf ("size %lu\n", sbuf.st_size);
  printf ("bklsize %lu\n", sbuf.st_blksize);
  printf ("blocks %lu\n", sbuf.st_blocks);
  printf ("atime %lu\n", sbuf.st_atime);
  printf ("mtime %lu\n", sbuf.st_mtime);
  printf ("ctime %lu\n", sbuf.st_ctime);

  if (lseek (fd, pos, SEEK_SET) == (off_t) -1)
    {
      FAILURE ("cannot seek %i (%s): %lu", fd, filename, pos);
    }

  char * buf = malloc (count);

  if (!buf)
    {
      FAILURE ("cannot allocate %lu bytes", count);
    }

  size_t left = count;

  while (left > 0)
    {
      ssize_t r = read (fd, buf, count);

      if (r < 0)
        {
          FAILURE ("cannot read %i (%s)", fd, filename);
        }

      left -= r;
    }

  printf ("read:\n");

  for (size_t i = 0; i < count; ++i)
    {
      printf ("%c", buf[i]);
    }

  printf ("\n");

  free (buf);

  if (close (fd) < 0)
    {
      FAILURE ("when closing file %s", filename);
    }
}

int
main ()
{
  run ("gpi/0/name", 2, 1);
  run ("gpi/0/data", (1<<20), (1<<20));

  printf ("SUCCESS\n");

  return EXIT_SUCCESS;
}
