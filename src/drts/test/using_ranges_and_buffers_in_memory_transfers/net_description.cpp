// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <drts/test/using_ranges_and_buffers_in_memory_transfers/net_description.hpp>

#include <util-generic/testing/random.hpp>

#include <boost/format.hpp>

#include <cmath>

namespace drts
{
  namespace test
  {
    std::string net_description
      ( std::string const& type
      , boost::optional<bool> allow_empty_ranges
      , bool with_alignment
      )
    {
      return
        (boost::format
          (R"EOS(<defun name="net">
            <include-structs href="memory/global/range.xpnet"/>
            <in name="global" type="global_memory_range" place="global"/>
            <in name="range_size" type="unsigned long" place="range_size"/>
            <in name="offset" type="unsigned long" place="offset"/>
            <in name="buffer_size" type="unsigned long" place="buffer_size"/>
            <out name="done" type="control" place="done"/>

            <net>
              <place name="global" type="global_memory_range"/>
              <place name="range_size" type="unsigned long"/>
              <place name="offset" type="unsigned long"/>
              <place name="buffer_size" type="unsigned long"/>
              <place name="done" type="control"/>

              <transition name="test_transition">
                <defun>
                  <in name="global" type="global_memory_range"/>
                  <in name="range_size" type="unsigned long"/>
                  <in name="offset" type="unsigned long"/>
                  <in name="buffer_size" type="unsigned long"/>
                  <out name="done" type="control"/>
                  <memory-buffer name="local">
                    <size>
                      ${buffer_size}
                    </size>
                    %1%
                  </memory-buffer>
                  <memory-%2%%3%>
                    <global>
                      ${range.handle} := ${global.handle};
                      ${range.offset} := 0UL;
                      ${range.size} := ${range_size};
                      stack_push (List(), ${range})
                    </global>
                    <local>
                      ${range.buffer} := "local";
                      ${range.offset} := ${offset};
                      ${range.size} := ${range_size};
                      stack_push (List(), ${range})
                    </local>
                  </memory-%2%>
                  <module name="test_module" function="done task (local)">
                    <cinclude href="stdexcept"/>
                    <code><![CDATA[
                      (void) local;
                      return we::type::literal::control();
                    ]]></code>
                  </module>
                </defun>
                <connect-in port="global" place="global"/>
                <connect-in port="range_size" place="range_size"/>
                <connect-in port="offset" place="offset"/>
                <connect-in port="buffer_size" place="buffer_size"/>
                <connect-out port="done" place="done"/>
              </transition>
            </net>
          </defun>)EOS")
          % ( with_alignment
            ? ( boost::format ("<alignment>%1%UL</alignment>")
              % std::pow (2, fhg::util::testing::random<std::size_t>{} (4, 0))
              ).str()
            : ""
            )
          % type
          % ( allow_empty_ranges
            ? ( boost::format (" allow-empty-ranges=\"%1%\"")
              % (*allow_empty_ranges ? "true" : "false")
              ).str()
            : ""
            )
          ).str();
    }
  }
}
