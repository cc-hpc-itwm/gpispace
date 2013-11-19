// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_RESERVATION_HPP
#define SDPA_RESERVATION_HPP 1

#include <boost/thread.hpp>
#include <sdpa/types.hpp>

namespace sdpa {
  namespace daemon {
    class Reservation : boost::noncopyable
    {
    public:
      typedef enum {FINISHED, FAILED, CANCELED} result_type;
      typedef std::map<sdpa::worker_id_t, result_type> map_worker_result_t;

      Reservation(const sdpa::job_id_t& job_id = "", const size_t& n=1) : m_job_id(job_id), m_capacity(n) {}

      size_t size() const { return m_list_workers.size(); }
      void addWorker(const sdpa::worker_id_t& wid) { m_list_workers.push_back(wid); }

      void storeWorkerResult(const sdpa::worker_id_t& wid, const result_type& result)
      {
        if(!hasWorker(wid))
        {
            std::string errMsg("No worker ");
            errMsg+=wid;
            errMsg+=" exists within the current reservation of the job ";
            errMsg+=m_job_id;
            throw std::runtime_error(errMsg);
        }
        else {
          // take the rseult and store it!
          map_worker_result_t::iterator it = m_map_worker_result.find(wid);
          if( it != m_map_worker_result.end() ) {
              it->second = result;
          } else {
              m_map_worker_result.insert(map_worker_result_t::value_type(wid, result));
          }
        }
      }

      void workerFinished(const sdpa::worker_id_t& wid) {
        storeWorkerResult(wid, FINISHED);
      }

      void workerFailed(const sdpa::worker_id_t& wid) {
        storeWorkerResult(wid, FAILED);
      }

      void workerCanceled(const sdpa::worker_id_t& wid) {
        storeWorkerResult(wid, CANCELED);
      }

      // should protect this!!!!
      bool allWorkersTerminated() const {return m_map_worker_result.size() == capacity(); }

      bool allGroupTasksFinishedSuccessfully()
      {
        for(map_worker_result_t::iterator it(m_map_worker_result.begin()); it!=m_map_worker_result.end(); it++)
          if(it->second!=FINISHED)
            return false;
        return true;
      }

      bool isEmpty() const { return m_list_workers.empty(); }
      bool acquired() const { return (size()==capacity()); }

      sdpa::worker_id_t headWorker() const { return m_list_workers.front(); }
      bool hasWorker(const sdpa::worker_id_t& wid) const { return find(m_list_workers.begin(), m_list_workers.end(), wid)!=m_list_workers.end(); }
      sdpa::worker_id_list_t getWorkerList() const { return m_list_workers; }

      size_t capacity() const { return m_capacity; }

      sdpa::job_id_t jobId() const { return m_job_id; }

    private:
      sdpa::job_id_t m_job_id;
      size_t m_capacity;
      sdpa::worker_id_list_t m_list_workers;
      map_worker_result_t m_map_worker_result;
    };
  }
}

#endif
