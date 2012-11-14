// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/base.hpp>

#include <fhg/util/backtracing_exception.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        base::base (change_manager_t& change_manager)
          : _change_manager (change_manager)
        { }

        void base::set_property ( const QObject* sender
                                , const ::we::type::property::key_type&
                                , const ::we::type::property::value_type&
                                ) const
        {
          throw fhg::util::backtracing_exception
            ("handle::base::set_property() called instead "
            "of overloaded version.");
        }

        change_manager_t& base::change_manager() const
        {
          return _change_manager;
        }
      }
    }
  }
}
