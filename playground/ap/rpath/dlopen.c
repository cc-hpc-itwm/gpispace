#include <dlfcn.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
	void *hdl;
	char *err;

	if (argc < 2)
	{
		fprintf (stderr, "usage: open <lib.so>\n");
		return 1;
	}

	dlerror ();

	hdl = dlopen (argv [1], RTLD_NOW);
	if (0 == hdl)
	{
		fprintf (stderr, "failed: %s\n", dlerror ());
		return 2;
	}

	dlclose (hdl);

	return 0;
}
