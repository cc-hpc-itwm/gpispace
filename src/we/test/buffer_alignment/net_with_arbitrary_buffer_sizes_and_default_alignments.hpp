#include <generate_buffer_names.hpp>

#include <util-generic/testing/random.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>

#include <string>

std::string net_with_arbitrary_buffer_sizes_and_default_alignments
  (unsigned long& total_buffer_size)
{
   std::string buffer_descriptions;
   std::string buffer_names;
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

     buffer_names += buffer_name + ",";

     alignment_tests +=
       ( boost::format (R"EOS(
            if (reinterpret_cast<std::uintptr_t> (%1%) %% %2%)
            {
              throw std::runtime_error ("Buffer not %2%-bytes aligned!");
            })EOS")
       % buffer_name
       % default_alignment
       ).str();
   }

   buffer_names.pop_back();

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
      % buffer_names
      % alignment_tests
      ).str()
    );

  return net_description;
}
