
#include "fread.h"

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

#define MIN(x,y) ((x)<(y)?(x):(y))

int read_file ( const int iPart
		         , const int nPart
		         , const char * filename
		         , void * buf
		         , const size_t buf_size
		         , const fvmAllocHandle_t hdl
		         )
{
	const fvmAllocHandle_t scratch = fvmLocalAlloc (buf_size);

	if (scratch == 0)
	{
		fprintf(stderr,"failed to alloc scratch handle of size %lu\n",buf_size);
		return -ENOMEM;
	}

	int fd = open (filename, O_RDONLY);

	if (fd == -1)
	{
		const int ec = errno;
		fprintf(stderr,"error opening filke %s: %s\n",filename, strerror(ec));
		return -ec;
	}
	else
	{
		struct stat sbuf;

		if (fstat (fd, &sbuf) == -1)
		{
			const int ec = errno;
			fprintf(stderr,"error in stat for %s: %s\n", filename, strerror(ec));
			return -ec;
		}
		else
		{
			const off_t first = ( iPart      * sbuf.st_size + nPart - 1) / nPart;
			const off_t last =  ((iPart + 1) * sbuf.st_size + nPart - 1) / nPart;

			if (lseek (fd, first, SEEK_SET) == (off_t) -1)
			{
				const int ec = errno;
				fprintf(stderr,"error seek in %s to %lu: %s\n", filename, first, strerror(ec));
				return -ec;
			}
			else
			{
				off_t left = last - first;

				while (left > 0)
				{
					const ssize_t r = read (fd, buf, MIN(buf_size,left));

					if (r == 0)
					{
						fprintf(stderr,"premature EOF in %s, %lu more bytes needed\n", filename, left);
						return -EIO;
					}
					else if (r == -1)
					{
						const int ec = errno;
						fprintf(stderr,"error reading from %s: %s\n", filename, strerror(ec));
						return -ec;
					}
					else
					{
						waitComm (fvmPutGlobalData (hdl, last - left, r, (char *)buf - (char *)fvmGetShmemPtr(), scratch));

					    left -= r;
					}
				}
			}
		}

		if (close (fd) == -1)
		{
			const int ec = errno;
		  fprintf(stderr,"error closing file %s: %s\n", filename, strerror(ec));
		  return -ec;
		}
	}

	return 0;
}
