#include <gspc/Task.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/print_container.hpp>

#include <boost/io/ios_state.hpp>

#include <iomanip>

namespace gspc
{
  std::ostream& operator<<
    (std::ostream& os, Task::SingleResource const& single_resource)
  {
    return os << "SingleResource {" << single_resource.first
              << ", " << single_resource.second
              << "}"
      ;
  }

  std::ostream& operator<<
    ( std::ostream& os
    , Task::SingleResourceWithPreference const& single_resource_with_preference
    )
  {
    return os << "SingleResourceWithPreference "
              << fhg::util::print_container
                   ( "[", ", ", "]"
                   , single_resource_with_preference
                   , [] (auto& s, auto const& x) -> decltype (s)
                     {
                       return s << x;
                     }
                   )
      ;
  }
  std::ostream& operator<<
    ( std::ostream& os
    , Task::SingleResourceWithCost const& single_resource_with_cost
    )
  {
    return os << "SingleResourceWithCost "
              << fhg::util::print_container
                   ("[", ", ", "]", single_resource_with_cost
                   , [] (auto& s, auto const& r) -> decltype (s)
                     {
                       return s << "{" << r.first
                                << ", " << r.second
                                << "}"
                         ;
                     }
                   )
      ;
  }
  std::ostream& operator<<
    ( std::ostream& os
    , Task::CoallocationSingleClass const& coallocation_single_class
    )
  {
    return os << "CoallocationSingleClass {"
              << coallocation_single_class.first
              << ", " << coallocation_single_class.second
              << "}"
      ;
  }
  std::ostream& operator<<
    ( std::ostream& os
    , Task::CoallocationSingleClassWithPreference const&
        coallocation_single_class_with_preference
    )
  {
    return os << "CoallocationSingleClassWithPreference "
              << fhg::util::print_container
                  ( "[", ", ", "]"
                  , coallocation_single_class_with_preference
                  , [] (auto& s, auto const& x) -> decltype (s)
                    {
                      return s << x;
                    }
                  );
  }

  std::ostream& operator<< (std::ostream& os, Task const& task)
  {
    os << "Task {"
       << "id = " << task.id
       << ", input = "
       << fhg::util::print_container
            ( "hex[", " ", "]", task.input
            , [&] (auto& s, auto const& x) -> decltype (s)
              {
                boost::io::ios_all_saver const save (s);

                return s << std::hex
                         << std::setw (2)
                         << std::setfill ('0')
                         << (static_cast<std::size_t> (x) & 0xFF);
              }
            )
       << ", requirement = "
      ;

    fhg::util::visit<void>
      (task.requirements, [&] (auto const& x)  { os << x; });

    return os << "}";
  }
}
