#include <stdint.h>
#include <inttypes.h>

extern int gpi_init ( int is_master
                    , const char *socket
                    , const char *plugin_dir
                    , const char *prep_wf
                    , const char *init_wf
                    , const char *calc_wf
                    , const char *mask_wf
                    , const char *done_wf
                    , const char *path_to_input
                    );

extern int gpi_wait (int rank);

extern int gpi_open (const char *);
extern int gpi_close (int);
extern long gpi_seek (const int, const size_t off, const int whence);
extern long gpi_read (int, void *buffer, size_t len);
extern long gpi_write (int, const void *buffer, size_t len);
