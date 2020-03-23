#include <generate_buffer_names.hpp>
#include <nets_using_buffers.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/align/is_aligned.hpp>
#include <boost/format.hpp>

#include <string>

std::string net_with_arbitrary_buffer_sizes_and_alignments
  (unsigned long& total_buffer_size)
{
  std::string buffer_descriptions;
  std::list<std::string> buffer_names;
  std::string alignment_tests;

  unsigned long num_buffers
    (fhg::util::testing::random<unsigned long>{} (15, 5));

  for (unsigned int i (0); i < num_buffers; ++i)
  {
    auto const buffer_name (get_new_buffer_name (buffer_names));
    auto const buffer_size
      (fhg::util::testing::random<unsigned long>{} (200, 100));

    auto const exp
      {fhg::util::testing::random<unsigned long>{}(10,0)};
    unsigned long const buffer_alignment (std::pow (2, exp));

    total_buffer_size += buffer_size + buffer_alignment - 1;

    buffer_descriptions +=
      ( boost::format (R"EOS(
       <memory-buffer name="%1%" readonly="true">
         <size>
           %2%UL
         </size>
         <alignment>
           %3%UL
         </alignment>
       </memory-buffer>)EOS")
      % buffer_name
      % buffer_size
      % buffer_alignment
      ).str();

    buffer_names.emplace_back (buffer_name);

    alignment_tests +=
      ( boost::format (R"EOS(
           if (!boost::alignment::is_aligned (%1%, %2%))
           {
             throw std::runtime_error ("Buffer not %1%-bytes aligned!");
           })EOS")
      % buffer_alignment
      % buffer_name
      ).str();
  }

  std::string const net_description
    ( ( boost::format (R"EOS(
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
      ).str()
    );

  return net_description;
}

std::string net_with_arbitrary_buffer_sizes_and_default_alignments
  (unsigned long& total_buffer_size)
{
  std::string buffer_descriptions;
  std::list<std::string> buffer_names;
  std::string alignment_tests;

  unsigned long const num_buffers
    (fhg::util::testing::random<unsigned long>{} (15, 5));

  unsigned int const default_alignment (1);

  for (unsigned int i (0); i < num_buffers; ++i)
  {
    auto const buffer_name (get_new_buffer_name (buffer_names));
    auto const buffer_size
      (fhg::util::testing::random<unsigned long>{} (200, 100));

    total_buffer_size += buffer_size;

    buffer_descriptions +=
      ( boost::format (R"EOS(
       <memory-buffer name="%1%" readonly="true">
         <size>
           %2%UL
         </size>
       </memory-buffer>)EOS")
      % buffer_name
      % buffer_size
      ).str();

    buffer_names.emplace_back (buffer_name);

    alignment_tests +=
      ( boost::format (R"EOS(
           if (!boost::alignment::is_aligned (%1%, %2%))
           {
             throw std::runtime_error ("Buffer not %1%-bytes aligned!");
           })EOS")
      % default_alignment
      % buffer_name
      ).str();
  }

  std::string const net_description
    ( ( boost::format (R"EOS(
<defun name="arbitrary_buffer_sizes_and_default_alignments">
  <in name="start" type="control" place="start"/>
  <out name="done" type="control" place="done"/>
  <net>
    <place name="start" type="control"/>
    <place name="done" type="control"/>
    <transition name="default_alignments">
      <defun>
        <in name="start" type="control"/>
        <out name="done" type="control"/>
        %1%
        <module name="default_alignments"
                function="done test (%2%)">
          <cinclude href="boost/align/is_aligned.hpp"/>
          <cinclude href="stdexcept"/>
          <cinclude href="inttypes.h"/>
          <cinclude href="iostream"/>
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
      ).str()
    );

  return net_description;
}

std::string net_with_arbitrary_buffer_sizes_and_alignments_insufficient_memory
  (unsigned long& total_buffer_size)
{
  std::string buffer_descriptions;
  std::list<std::string> buffer_names;
  std::string alignment_tests;

  unsigned long num_buffers
    (fhg::util::testing::random<unsigned long>{} (15, 5));

  for (unsigned int i (0); i < num_buffers; ++i)
  {
    auto const buffer_name (get_new_buffer_name (buffer_names));
    auto const buffer_size
      (fhg::util::testing::random<unsigned long>{} (200, 100));

    auto const exp
      {fhg::util::testing::random<unsigned long>{} (10, 0)};
    unsigned long const buffer_alignment (std::pow (2, exp));

    total_buffer_size += buffer_size;

    buffer_descriptions +=
      ( boost::format (R"EOS(
       <memory-buffer name="%1%" readonly="true">
         <size>
           %2%UL
         </size>
         <alignment>
           %3%UL
         </alignment>
       </memory-buffer>)EOS")
      % buffer_name
      % buffer_size
      % buffer_alignment
      ).str();

    buffer_names.emplace_back (buffer_name);

    alignment_tests +=
      ( boost::format (R"EOS(
           if (!boost::alignment::is_aligned (%1%, %2%))
           {
             throw std::runtime_error ("Buffer not %1%-bytes aligned!");
           })EOS")
      % buffer_alignment
      % buffer_name
      ).str();
  }

  std::string const net_description
    ( ( boost::format (R"EOS(
<defun name="arbitrary_buffer_sizes_and_alignments_insufficient_memory">
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
          <cinclude href="boost/align/is_aligned.hpp"/>
          <cinclude href="stdexcept"/>
          <cinclude href="inttypes.h"/>
          <cinclude href="iostream"/>
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
      ).str()
    );

  return net_description;
}
