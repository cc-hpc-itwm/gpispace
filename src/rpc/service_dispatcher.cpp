#include <gspc/rpc/service_dispatcher.hpp>

#include <gspc/rpc/common.hpp>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

#include <stdexcept>


  namespace gspc::rpc
  {
    service_dispatcher::service_dispatcher
        (gspc::util::serialization::exception::serialization_functions functions)
          : _serialization_functions (error::add_builtin (std::move (functions)))
    {}

    void service_dispatcher::dispatch ( ::boost::asio::yield_context yield
                                      , ::boost::archive::binary_iarchive& input
                                      , ::boost::archive::binary_oarchive& output
                                      ) const
    {
      std::string function;
      input >> function;

      try
      {
        decltype (_handlers)::const_iterator const handler
          (_handlers.find (function));

        if (handler == _handlers.end())
        {
          throw error::unknown_function (function);
        }

        handler->second (yield, input, output);
      }
      catch (...)
      {
        output << true;
        gspc::util::serialization::exception::serialize
          (output, std::current_exception(), _serialization_functions);
      }
    }
  }
