#include <fhgcom/io_service_pool.hpp>

#include <boost/thread.hpp>

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
      boost::thread_group  threads;
        for (std::size_t j (0) ; j < m_nthreads ; ++j)
        {
          threads.create_thread ([this] { io_service_->run(); });
        }

        threads.join_all();
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
