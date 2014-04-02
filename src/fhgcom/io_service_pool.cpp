#include <fhgcom/io_service_pool.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <stdexcept>

namespace fhg
{
  namespace com
  {
    io_service_pool::io_service_pool ()
      : io_service_ (new boost::asio::io_service)
      , work_ (new boost::asio::io_service::work (*io_service_))
      , m_nthreads (1)
    {}

    void io_service_pool::run ()
    {
      std::vector<boost::shared_ptr<boost::thread>> threads;
        for (std::size_t j (0) ; j < m_nthreads ; ++j)
        {
          boost::shared_ptr<boost::thread> thread
            (new boost::thread([this] { io_service_->run(); }));
          threads.push_back (thread);
        }

      for (std::size_t i (0); i < threads.size(); ++i)
      {
        threads[i]->join();
      }
    }

    void io_service_pool::stop ()
    {
        io_service_->stop();
    }

    boost::asio::io_service & io_service_pool::get_io_service ()
    {
      return *io_service_;
    }
  }
}
