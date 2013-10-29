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

      bool allWorkersTerminated() const { return m_map_worker_result.size() == m_list_workers.size(); }
      bool isEmpty() const { return m_map_worker_result.empty(); }
      worker_id_t headWorker() const { return m_list_workers.front(); }
      bool hasWorker(const worker_id_t& wid) const { return find(m_list_workers.begin(), m_list_workers.end(), wid)!=m_list_workers.end(); }
      worker_id_list_t getWorkerList() const { return m_list_workers; }

    private:
      job_id_t m_group_job_id;
      worker_id_list_t m_list_workers;
      map_worker_result_t m_map_worker_result;
    };
  }
}

#endif
