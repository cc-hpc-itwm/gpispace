// Copyright (C) 2020-2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/we/buffer_alignment/net_description.hpp>

#include <gspc/we/type/value/show.hpp>

#include <gspc/util/print_container.hpp>

#include <gspc/util/join.formatter.hpp>
#include <gspc/we/type/value/show.formatter.hpp>
#include <fmt/core.h>



    namespace gspc::we::test::buffer_alignment
    {
      namespace
      {
        std::string create_buffer_description (BufferInfo const& buffer)
        {
          if (!buffer.alignment)
          {
            return fmt::format (R"EOS(
                     <memory-buffer name="{0}" readonly="true">
                       <size>
                         {1}
                       </size>
                     </memory-buffer>)EOS"
                   , buffer.name
                   , gspc::pnet::type::value::show (buffer.size)
                   );
          }
          else
          {
            return fmt::format (R"EOS(
                     <memory-buffer name="{0}" readonly="true">
                       <size>
                         {1}
                       </size>
                       <alignment>
                         {2}
                       </alignment>
                     </memory-buffer>)EOS"
                   , buffer.name
                   , gspc::pnet::type::value::show (buffer.size)
                   , gspc::pnet::type::value::show (*buffer.alignment)
                   );
          }
        }

        std::string create_alignment_test (BufferInfo const& buffer)
        {
          return fmt::format (R"EOS(
                   if (!is_aligned ({0}, {1}))
                   {{
                     throw std::runtime_error ("Buffer not {0}-bytes aligned!");
                   }})EOS"
                 , buffer.alignment.value_or (1)
                 , buffer.name
                 );
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

        return fmt::format (R"EOS(
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
                         {0}
                         <module name="arbitrary_alignments"
                                 function="done test ({1})">
                           <cinclude href="stdexcept"/>
                           <cinclude href="inttypes.h"/>
                           <cinclude href="iostream"/>
                           <cxx flag="--std=c++17"/>
                           <code><![CDATA[
                             auto is_aligned
                               {{ [] (std::size_t alignment, void const* ptr)
                                 {{
                                   auto const value {{(std::size_t)ptr}};
                                   return (value & (alignment - 1)) == 0;
                                 }}
                               }};
                             {2}
                             return gspc::we::type::literal::control();
                           ]]></code>
                         </module>
                       </defun>
                       <connect-in port="start" place="start"/>
                       <connect-out port="done" place="done"/>
                     </transition>
                   </net>
                 </defun>)EOS"
               , buffer_descriptions
               , gspc::util::print_container ("", ",", "", buffer_names)
               , alignment_tests
               );
      }
    }
