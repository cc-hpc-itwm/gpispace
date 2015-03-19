// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/print_exception.hpp>

#include <util-generic/ndebug.hpp>

#include <typeinfo>

namespace fhg
{
  namespace util
  {
    namespace
    {
      void print_unknown ( std::ostream& os
                         , std::string const& prefix
                         , int indentation
                         )
      {
        os << std::string (indentation, ' ') << prefix
           << "unknown exception type\n";
      }
    }

    void print_exception ( std::ostream& os
                         , std::string const& prefix
                         , std::exception const& e
                         , int indentation
                         )
    {
      os << std::string (indentation, ' ') << prefix
        IFNDEF_NDEBUG (<< typeid (e).name() << ": ")
         << e.what() << '\n';
      try
      {
        std::rethrow_if_nested (e);
      }
      catch (std::exception const& e)
      {
        print_exception (os, prefix, e, indentation + 1);
      }
      catch (...)
      {
        print_unknown (os, prefix, indentation + 1);
      }
    }

    void print_exception ( std::ostream& os
                         , std::string const& prefix
                         , std::exception_ptr const& e
                         , int indentation
                         )
    {
      try
      {
        std::rethrow_exception (e);
      }
      catch (std::exception const& e)
      {
        print_exception (os, prefix, e, indentation);
      }
      catch (...)
      {
        print_unknown (os, prefix, indentation);
      }
    }

    void print_current_exception ( std::ostream& os
                                 , std::string const& prefix
                                 , int indentation
                                 )
    {
      print_exception (os, prefix, std::current_exception(), indentation);
    }
  }
}
