// explicit template instantiation file
// this file is only compiled when NO_IMPLICIT_TEMPLATES is set to true

#include <boost/program_options.hpp>

template boost::program_options::typed_value<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, char>* boost::program_options::value<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >(std::basic_string<char, std::char_traits<char>, std::allocator<char> >*);
template boost::program_options::basic_parsed_options<char> boost::program_options::parse_command_line<char>(int, char**, boost::program_options::options_description const&, int, boost::function1<std::pair<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&>);

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <fhgcom/io_service_pool.hpp>
template class boost::_bi::bind_t< void
                                 , boost::_mfi::mf0<void, fhg::com::io_service_pool>
                                 , boost::_bi::list1<boost::_bi::value<fhg::com::io_service_pool*> >
                                 >;

#include <string>
template std::basic_string<char, std::char_traits<char>, std::allocator<char> > boost::detail::lexical_cast<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::basic_string<char, std::char_traits<char>, std::allocator<char> >, false, char>(boost::call_traits<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >::param_type, char*, unsigned long);

#include <map>
template class std::_Rb_tree<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::pair<std::basic_string<char, std::char_traits<char>, std::allocator<char> > const, boost::program_options::variable_value>, std::_Select1st<std::pair<std::basic_string<char, std::char_traits<char>, std::allocator<char> > const, boost::program_options::variable_value> >, std::less<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::basic_string<char, std::char_traits<char>, std::allocator<char> > const, boost::program_options::variable_value> > >;

#include <boost/shared_ptr.hpp>
template boost::_bi::bind_t<void, boost::_mfi::mf0<void, fhg::com::io_service_pool>, boost::_bi::list1<boost::_bi::value<fhg::com::io_service_pool*> > >::bind_t(boost::detail::thread_move_t<boost::_bi::bind_t<void, boost::_mfi::mf0<void, fhg::com::io_service_pool>, boost::_bi::list1<boost::_bi::value<fhg::com::io_service_pool*> > > >&);
template class boost::detail::sp_counted_impl_p<boost::detail::thread_data<boost::_bi::bind_t<void, boost::_mfi::mf0<void, fhg::com::io_service_pool>, boost::_bi::list1<boost::_bi::value<fhg::com::io_service_pool*> > > > >;
template class boost::detail::thread_data<boost::_bi::bind_t<void, boost::_mfi::mf0<void, fhg::com::io_service_pool>, boost::_bi::list1<boost::_bi::value<fhg::com::io_service_pool*> > > >;
