// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <drts/test/using_ranges_and_buffers_in_memory_transfers/net_description.hpp>

#include <util-generic/testing/random.hpp>

#include <fmt/core.h>
#include <cmath>

namespace drts
{
  namespace test
  {
    std::string net_description
      ( std::string const& type
      , ::boost::optional<bool> allow_empty_ranges
      , bool with_alignment
      )
    {
      return fmt::format
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
                      ${{buffer_size}}
                    </size>
                    {0}
                  </memory-buffer>
                  <memory-{1}{2}>
                    <global>
                      ${{range.handle}} := ${{global.handle}};
                      ${{range.offset}} := 0UL;
                      ${{range.size}} := ${{range_size}};
                      stack_push (List(), ${{range}})
                    </global>
                    <local>
                      ${{range.buffer}} := "local";
                      ${{range.offset}} := ${{offset}};
                      ${{range.size}} := ${{range_size}};
                      stack_push (List(), ${{range}})
                    </local>
                  </memory-{1}>
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
          </defun>)EOS"
         , ( with_alignment
            ? fmt::format
              ( "<alignment>{}UL</alignment>"
              , std::pow (2, fhg::util::testing::random<std::size_t>{} (4, 0))
              )
            : ""
            )
          , type
          , ( allow_empty_ranges
            ? fmt::format
              ( " allow-empty-ranges=\"{}\""
              , (*allow_empty_ranges ? "true" : "false")
              )
            : ""
            )
          );
    }
  }
}
