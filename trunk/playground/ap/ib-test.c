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
  long size;

  if (ac < 2)
  {
    fprintf(stderr, "usage: %s #bytes\n", av[0]);
    return EXIT_FAILURE;
  }

  size = atoll (av[1]);

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

  // open all devices
  printf ("opening devices...\n");
  struct ibv_context ** context = malloc(num_devices * sizeof(struct ibv_device*));

  for (i = 0; i < num_devices; ++i)
  {
    context[i] = ibv_open_device(device[i]);
    if (! context[i])
    {
      printf ("could not open device: %s\n", ibv_get_device_name(device[i]));
    }
  }

  printf ("allocating protection domain\n");
  struct ibv_pd *pd = ibv_alloc_pd(context[0]);

  printf ("registering memory...\n");

  enum ibv_access_flags flags =
    IBV_ACCESS_LOCAL_WRITE
    | IBV_ACCESS_REMOTE_WRITE
    | IBV_ACCESS_REMOTE_READ;

  struct ibv_mr * mr = ibv_reg_mr (pd, mem, size, flags);

  if (0 == mr)
  {
    printf ("failed: ibv_reg_mr(%p, %p, %lld, %x): %s\n", pd,mem,size,flags,strerror(errno));
    return 5;
  }
  else
  {
    printf ("registered memory of size %lld\n", size);
  }

  ibv_dereg_mr (mr);

  ibv_dealloc_pd (pd);

  for (i = 0; i < num_devices; ++i)
  {
    ibv_close_device (context[i]);
  }
  free (context);

  ibv_free_device_list (device);
  device = 0;

  free (mem);

  return EXIT_SUCCESS;
}
