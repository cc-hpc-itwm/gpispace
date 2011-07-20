#ifndef DRTS_PLUGIN_HPP
#define DRTS_PLUGIN_HPP 1

#include <string>

namespace drts
{
  typedef std::string job_desc_t;
  typedef std::string job_id_t;

  namespace status
  {
    enum status_t
      {
        PENDING = 0
        , RUNNING = 1
        , FINISHED = 2
        , PAUSED = 3
        , CANCELED = 4
        , FAILED = 5
      };
  }

  class JobListener
  {
  public:
    virtual void jobStatusChanged(job_id_t const &, drts::status::status_t) = 0;
  };

  class DRTS
  {
  public:
    virtual int exec (job_desc_t const &, job_id_t &, JobListener *) = 0;
    virtual status::status_t query (job_id_t const & jobid) = 0;
    virtual int cancel (job_id_t const & jobid) = 0;
    virtual int results (job_id_t const & jobid, std::string &) = 0;
    virtual int remove (job_id_t const & jobid) = 0;
    // virtual capabilities_t capabilities() const = 0;
  };
}

#endif
