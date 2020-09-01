#include <iml/vmem/netdev_id.hpp>

#include <fhg/util/boost/program_options/validators.hpp>

#include <boost/program_options/errors.hpp>

namespace fhg
{
  namespace iml
  {
    namespace vmem
    {
      netdev_id::netdev_id()
        : netdev_id ("auto")
      {}

      netdev_id::netdev_id (std::string const& option)
        : value (option == "0" ? 0 : option == "1" ? 1 : -1)
      {
        if (option != "auto" && value != 0 && value != 1)
        {
          throw boost::program_options::invalid_option_value
            ("Expected 'auto' or '0' or '1', got '" + option + "'");
        }
      }

      std::string to_string (netdev_id const& id)
      {
        return id.value == -1 ? "auto" : std::to_string (id.value);
      }

      std::ostream& operator<< (std::ostream& os, netdev_id const& id)
      {
        return os << to_string (id);
      }

      void validate
        (boost::any& v, std::vector<std::string> const& values, netdev_id*, int)
      {
        fhg::util::boost::program_options::validate<netdev_id> (v, values);
      }
    }
  }
}
