#ifndef NODE_MANAGEMENT_DAEMON_H
#define NODE_MANAGEMENT_DAEMON_H 1

#ifdef __cplusplus
#define BEGIN_C_DECLS extern "C" {
#define END_C_DECLS }
#else
#define BEGIN_C_DECLS
#define END_C_DECS
#endif

BEGIN_C_DECLS

/*
  set the key required to communicate
 */
int nmd_set_key (const char * bytes, size_t len);

/*
  return 0 on success
  -errno otherwise
*/
int nmd_check_daemon (const char * host);

typedef struct
{
  int uid;
  int gid;
} nmd_process_caps_t;

enum nmd_redirect_type_t
  {
    NMD_STREAM       /* stream the redirected file descriptor over the network */
    , NMD_FILE       /* redirect to some path */
  };

struct nmd_redirect_t
{
  int           fd;                  /* which filedescriptor to redirect */
  enum          nmd_redirect_type_t; /* how to redirect */
  unsigned long value;               /* where to redirect (eg. "/dev/null" or 0 in case of STREAM) */
};

typedef struct
{
  int                 argc;
  char                **argv;
  char                **environ;
  nmd_process_caps_t  caps;
  nmd_redirect_t      *redirect;
} nmd_process_data_t;

typedef int nmd_pid_t;
nmd_pid_t nmd_exec (const char * host, const nmd_process_t *);

/*
  similar behavior as kill(2):
      if pid > 0   : send sig to pid
      if pid == 0  : send sig to all processes started by the current process
      if pid == -1 : send signal to all processes
      if pid < -1  : EINVAL
      if sig == 0  : just check existence of pid
*/
int nmd_kill (const nmd_pid_t pid, int sig);



/*****************************************
 *          optional functions           *
 *****************************************/

/* returns the number of written bytes or -errno */
int nmd_write ( const nmd_pid_t pid   /* the process in question */
              , int fd                /* must be one with NMD_STREAM type */
              , const void * buf      /* data to send */
              , size_t len            /* how much data */
              );

int nmd_read ( const nmd_pid_t pid
             , int fd
             , void * buf
             , size_t len
             );

typedef struct { } nmd_node_info_t;
/*
  get information about a node
    - current load
    - memory  consumption
    - capabilities...
 */
int nmd_get_node_info (const char *, nmd_node_info_t *);

/* put and get key/value pairs */

typedef struct
{
  int ttl;      /* maximum number of seconds the key shall exist 0 - forever */
  int flags;    /* remove key on disconnect and so on */
  int type;     /* could be used to handle special value types (list value: put results in append) */
} nmd_key_caps_t;

int nmd_put_data(const char *key, nmd_key_caps_t*, const void *value, size_t);
int nmd_get_data(const char *key, void *value, size_t);

END_C_DECLS

#endif
