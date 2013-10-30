// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_RESERVATION_HPP
#define SDPA_RESERVATION_HPP 1

namespace sdpa {
  namespace daemon {
    class Reservation
    {
    public:
      typedef enum {FINISHED, FAILED, CANCELED} result_type;
      typedef std::map<worker_id_t, result_type> map_worker_result_t;

      Reservation(const job_id_t& job_id = "", const size_t& n=1) : m_job_id(job_id), m_capacity(n) {}
      ~Reservation() { m_list_workers.clear(); m_map_worker_result.clear(); }

      size_t size() { return m_list_workers.size(); }
      void addWorker(const worker_id_t& wid) { m_list_workers.push_back(wid); }
      void storeWorkerResult(const worker_id_t& wid, const result_type& result) {
        map_worker_result_t::iterator it = m_map_worker_result.find(wid);
        if( it != m_map_worker_result.end() ) {
            it->second = result;
        } else {
            m_map_worker_result.insert(map_worker_result_t::value_type(wid, result));
        }
      }

      void workerFinished(const worker_id_t& wid) {
        storeWorkerResult(wid, FINISHED);
      }

      void workerFailed(const worker_id_t& wid) {
        storeWorkerResult(wid, FAILED);
      }

      void workerCanceled(const worker_id_t& wid) {
        storeWorkerResult(wid, CANCELED);
      }

      // should protect this!!!!
      bool allWorkersTerminated() const { return m_map_worker_result.size() == capacity(); }

      bool groupFinished()
      {
        for(map_worker_result_t::iterator it(m_map_worker_result.begin()); it!=m_map_worker_result.end(); it++)
          if(it->second!=FINISHED)
            return false;
        return true;
      }

      bool isEmpty() const { return m_list_workers.empty(); }
      bool acquired() { return (size()==capacity()); }

      worker_id_t headWorker() const { return m_list_workers.front(); }
      bool hasWorker(const worker_id_t& wid) const { return find(m_list_workers.begin(), m_list_workers.end(), wid)!=m_list_workers.end(); }
      worker_id_list_t getWorkerList() const { return m_list_workers; }

      size_t capacity() const { return m_capacity; }

      job_id_t const jobId() { return m_job_id; }

    private:
      size_t m_capacity;
      job_id_t m_job_id;
      worker_id_list_t m_list_workers;
      map_worker_result_t m_map_worker_result;
    };
  }
}

#endif
