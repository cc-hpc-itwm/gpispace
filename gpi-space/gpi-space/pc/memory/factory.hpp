#ifndef GPI_SPACE_PC_MEMORY_FACTORY_HPP
#define GPI_SPACE_PC_MEMORY_FACTORY_HPP

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class area_t;
      typedef boost::shared_ptr<area_t> area_ptr_t;
      typedef boost::function<area_ptr_t (std::string const &)> factory_function_t;

      class factory_t
      {
        typedef std::map<std::string, factory_function_t> function_map_t;
      public:
        void register_type ( std::string const &type
                           , factory_function_t fun
                           );
        void unregister_type (std::string const &type);

        area_ptr_t create (std::string const & url);
      private:
        function_map_t m_factory_functions;
      };

      factory_t & factory ();
    }
  }
}

#endif
