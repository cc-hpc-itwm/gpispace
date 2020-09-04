// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/test/buffer_alignment/net_description.hpp>

#include <we/type/value/show.hpp>

#include <util-generic/print_container.hpp>

#include <boost/format.hpp>

namespace we
{
  namespace test
  {
    namespace buffer_alignment
    {
      namespace
      {
        std::string create_buffer_description (BufferInfo const& buffer)
        {
          if (!buffer.alignment)
          {
            return ( boost::format (R"EOS(
                     <memory-buffer name="%1%" readonly="true">
                       <size>
                         %2%
                       </size>
                     </memory-buffer>)EOS")
                   % buffer.name
                   % pnet::type::value::show (buffer.size)
                   ).str();
          }
          else
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
                   % buffer.name
                   % pnet::type::value::show (buffer.size)
                   % pnet::type::value::show (*buffer.alignment)
                   ).str();
          }
        }

        std::string create_alignment_test (BufferInfo const& buffer)
        {
          return ( boost::format (R"EOS(
                   if (!boost::alignment::is_aligned (%1%, %2%))
                   {
                     throw std::runtime_error ("Buffer not %1%-bytes aligned!");
                   })EOS")
                 % buffer.alignment.get_value_or (1)
                 % buffer.name
                 ).str();
        }
      }

      std::string create_net_description
        (std::vector<BufferInfo> const& buffers)
      {
        std::string buffer_descriptions;
        std::string alignment_tests;
        std::vector<std::string> buffer_names;

        for (auto const& buffer : buffers)
        {
          buffer_descriptions += create_buffer_description (buffer);
          alignment_tests += create_alignment_test (buffer);
          buffer_names.emplace_back (buffer.name);
        }

        return ( boost::format (R"EOS(
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
                 </defun>)EOS")
               % buffer_descriptions
               % fhg::util::print_container ("", ",", "", buffer_names)
               % alignment_tests
               ).str();
      }
    }
  }
}
