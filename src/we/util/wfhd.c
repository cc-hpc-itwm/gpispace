#include "wfhd.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

// http://beej.us/guide/bgnet/output/html/multipage/clientserver.html#simpleserver

// get sockaddr, IPv4 or IPv6:
static void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

#define BACKLOG 1

static int s_wfhd_server_port = -1;

int wfhd_wait (const char *wfhd_info_path)
{
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, "0", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                  sizeof(int)) == -1) {
      int ec = errno;
      perror("setsockopt");
      close (sockfd);
      errno = ec;
      return -1;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    return 2;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (listen(sockfd, BACKLOG) == -1) {
    int ec = errno;
    perror("listen");
    close (sockfd);
    errno = ec;
    return -1;
  }

  struct sockaddr_in sin;
  sin_size = sizeof(sin);
  if (getsockname (sockfd, (struct sockaddr *)&sin, &sin_size) == -1)
  {
    int ec = errno;
    perror ("getsockname");
    errno = ec;
    return -1;
  }
  else
  {
    FILE *wfhd_info = fopen (wfhd_info_path, "w");
    if (wfhd_info == 0)
    {
      int ec = errno;
      fprintf ( stderr
              , "could not open %s for writing: %s\n"
              , wfhd_info_path
              , strerror (errno)
              );
      close (sockfd);
      errno = ec;
      return -1;
    }
    else
    {
      char buf [1024];
      gethostname (buf, sizeof (buf));
      s_wfhd_server_port = ntohs (sin.sin_port);
      fprintf (wfhd_info, "%s %d\n", buf, ntohs (sin.sin_port));
      fclose (wfhd_info); wfhd_info = NULL;
    }
  }

  {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1)
    {
      s_wfhd_server_port = -1;
      int ec = errno;
      perror("accept");
      fflush (stderr);
      close (sockfd);
      errno = ec;
      return -1;
    }

    inet_ntop(their_addr.ss_family,
             get_in_addr((struct sockaddr *)&their_addr),
             s, sizeof s);
    fflush (stdout);
    close(new_fd);  // parent doesn't need this
  }

  s_wfhd_server_port = -1;
  close (sockfd);

  unlink (wfhd_info_path);

  return 0;
}

static int s_wfhd_signal (const char *hostname, const char *service)
{
  int sockfd;  // client socket
  struct addrinfo hints, *servinfo, *p;
  int rv;

  fprintf (stderr, "signalling %s %s\n", hostname, service);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(hostname, service, &hints, &servinfo)) != 0)
  {
    int ec = errno;
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    errno = ec;
    return -1;
  }

  // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    errno = ECONNREFUSED;
    return -1;
  }

  freeaddrinfo(servinfo); // all done with this structure
  close(sockfd);

  return 0;
}

int wfhd_signal (const char *wfhd_info_path)
{
  char hostname [4096];
  char service [1024];

  // read server information from file
  {
    FILE *wfhd_info = fopen (wfhd_info_path, "r");
    if (wfhd_info == NULL)
    {
      int ec = errno;
      fprintf ( stderr, "could not open %s for reading: %s\n"
              , wfhd_info_path
              , strerror (errno)
              );
      errno = ec;
      return -1;
    }

    if (fscanf (wfhd_info, "%s %s", hostname, service) != 2)
    {
      int ec = errno;
      fprintf ( stderr, "could not read 'host service': %s\n"
              , strerror (errno)
              );
      errno = ec;
      fclose (wfhd_info);
      return -1;
    }

    fclose (wfhd_info);
  }

  return s_wfhd_signal (hostname, service);
}

void wfhd_cancel (void *dummy)
{
  (void)(dummy);
  if (s_wfhd_server_port >= 0)
  {
    char service [16];
    snprintf (service, sizeof(service), "%d", s_wfhd_server_port);
    if (s_wfhd_signal ("localhost", service) != 0)
    {
      fprintf ( stderr
              , "cancel failed: port %s : %s\n", service, strerror (errno)
              );
      fflush (stderr);
    }
  }
}

