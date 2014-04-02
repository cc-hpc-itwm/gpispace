#include <fhgcom/io_service_pool.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <stdexcept>

namespace fhg
{
  namespace com
  {
    io_service_pool::io_service_pool ()
      : m_nthreads (1)
    {
        io_service_ptr io_service (new boost::asio::io_service);
        work_ptr work (new boost::asio::io_service::work (*io_service));
        io_services_.push_back (io_service);
        work_.push_back (work);
    }

    void io_service_pool::run ()
    {
      std::vector<boost::shared_ptr<boost::thread>> threads;
        for (std::size_t j (0) ; j < m_nthreads ; ++j)
        {
          boost::shared_ptr<boost::thread> thread
            (new boost::thread([this] { io_services_[0]->run(); }));
          threads.push_back (thread);
        }

      for (std::size_t i (0); i < threads.size(); ++i)
      {
        threads[i]->join();
      }
    }

    void io_service_pool::stop ()
    {
        io_services_[0]->stop();
    }

    boost::asio::io_service & io_service_pool::get_io_service ()
    {
      boost::asio::io_service & io_service = *io_services_[0];
      return io_service;
    }
  }
}
