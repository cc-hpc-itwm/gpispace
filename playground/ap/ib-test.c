#include <stdlib.h>
#include <malloc.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>

#include <infiniband/verbs.h>

int main(int ac, char *av[])
{
  struct ibv_device ** device;
  int num_devices;
  int i;
  int rc;
  long long size;
  int dev;

  if (ac < 2)
  {
    fprintf(stderr, "usage: %s #bytes [device-id]\n", av[0]);
    return EXIT_FAILURE;
  }

  if (ac > 2)
  {
    dev = atoi(av[2]);
    if (dev < 0)
    {
      fprintf(stderr, "invalid device, must be nonzero\n");
      return EXIT_FAILURE;
    }
  }
  else
  {
    dev = 0;
  }

  size = atoll (av[1]);

  printf ("page-size = %d\n", sysconf(_SC_PAGESIZE));

  printf ("allocating %lld bytes...\n", size);

  void *mem = memalign(sysconf(_SC_PAGESIZE), size);
  if (0 == mem)
  {
    printf ("failed to allocate aligned memory: %s\n", strerror(errno));
    return 2;
  }

  printf ("locking memory at %p\n", mem);
  if (mlock(mem,size) != 0)
  {
    printf("failed: mlock(%p, %lld): %s\n", mem, size, strerror(errno));
    printf("  hint: check ulimit -l\n");
    printf("  hint: it should be set to unlimited\n");
    return 3;
  }

  printf ("searching devices...\n");

  device = ibv_get_device_list(&num_devices);

  if (num_devices == 0)
  {
    printf ("no device found!\n");
    return 4;
  }

  printf ("found %d devices:\n", num_devices);

  for (i = 0; i < num_devices; ++i)
  {
    printf ("\tdevice-%d : %s\n", i, ibv_get_device_name(device[i]));
  }

  if (dev >= num_devices)
  {
    fprintf(stderr, "device %d is out-of-range!\n", dev);
    return EXIT_FAILURE;
  }

  printf ("opening device %d...\n", dev);

  struct ibv_context * context = ibv_open_device(device[dev]);
  if (0 == context)
  {
    fprintf (stderr, "could not open device: %s\n", ibv_get_device_name(device[dev]));
    return EXIT_FAILURE;
  }

  struct ibv_device_attr dev_attr;
  rc = ibv_query_device (context, &dev_attr);
  if (rc != 0)
  {
    fprintf (stderr, "could not query device: ibv_query_device(%x, %x): %s"
            , context
            , &dev_attr
            , strerror(rc)
            );
    return EXIT_FAILURE;
  }

  printf ("device info:\n");
  printf ("\tmax_mr      = %d\n", dev_attr.max_mr);
  printf ("\tmax_mr_size = %llu\n", dev_attr.max_mr_size);

  if (size > dev_attr.max_mr_size)
  {
    printf("*** your specified size exeeds the max size of one registered memory block\n");
  }

  printf ("allocating protection domain\n");
  struct ibv_pd *pd = ibv_alloc_pd(context);

  printf ("registering memory...\n");

  enum ibv_access_flags flags =
    IBV_ACCESS_LOCAL_WRITE
    | IBV_ACCESS_REMOTE_WRITE
    | IBV_ACCESS_REMOTE_READ;

  struct ibv_mr * mr = ibv_reg_mr (pd, mem, size, flags);

  if (0 == mr)
  {
    printf ("failed: ibv_reg_mr(%p, %p, %llu, %x): %s\n", pd,mem,size,flags,strerror(errno));
    printf ("   hint: try to check the parameter 'log_mtts_per_seg' of the mlx4_core kernel module\n");
    return 5;
  }
  else
  {
    printf ("registered memory of size %lld\n", size);
  }

  ibv_dereg_mr (mr);

  ibv_dealloc_pd (pd);

  ibv_close_device (context);

  ibv_free_device_list (device);

  free (mem);

  printf ("check successful\n");

  return EXIT_SUCCESS;
}
