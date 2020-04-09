#include <net_description.hpp>

#include <we/type/value/show.hpp>
#include <util-generic/print_container.hpp>

#include <boost/format.hpp>

std::string create_buffer_description
  ( std::string const& name
  , std::size_t size
  )
{
  return ( boost::format (R"EOS(
       <memory-buffer name="%1%" readonly="true">
         <size>
           %2%
         </size>
       </memory-buffer>)EOS")
         % name
         % pnet::type::value::show (size)
         ).str();
}

std::string create_buffer_description
  ( std::string const& name
  , std::size_t size
  , std::size_t alignment
  )
{
  return ( boost::format (R"EOS(
       <memory-buffer name="%1%" readonly="true">
         <size>
           %2%
         </size>
         <alignment>
           %3%
         </alignment>
       </memory-buffer>)EOS")
         % name
         % pnet::type::value::show (size)
         % pnet::type::value::show (alignment)
         ).str();
}

std::string create_alignment_test
  ( std::size_t alignment
  , std::string const& name
  )
{
  return ( boost::format (R"EOS(
           if (!boost::alignment::is_aligned (%1%, %2%))
           {
             throw std::runtime_error ("Buffer not %1%-bytes aligned!");
           })EOS")
         % alignment
         % name
         ).str();
}

std::string create_net_description
  ( std::string const& buffer_descriptions
  , std::list<std::string> const& buffer_names
  , std::string const& alignment_tests
  )
{
  return
    ( boost::format (R"EOS(
<defun name="arbitrary_buffer_sizes_and_alignments">
  <in name="start" type="control" place="start"/>
  <out name="done" type="control" place="done"/>
  <net>
    <place name="start" type="control"/>
    <place name="done" type="control"/>
    <transition name="arbitrary_alignments">
      <defun>
        <in name="start" type="control"/>
        <out name="done" type="control"/>
        %1%
        <module name="arbitrary_alignments"
                function="done test (%2%)">
          <cinclude href="stdexcept"/>
          <cinclude href="inttypes.h"/>
          <cinclude href="iostream"/>
          <cinclude href="boost/align/is_aligned.hpp"/>
          <cxx flag="--std=c++11"/>
          <code><![CDATA[
            %3%
            return we::type::literal::control();
          ]]></code>
        </module>
      </defun>
      <connect-in port="start" place="start"/>
      <connect-out port="done" place="done"/>
    </transition>
  </net>
</defun>
  )EOS")
      % buffer_descriptions
      % fhg::util::print_container ("", ",", "", buffer_names)
      % alignment_tests
      ).str();
}
