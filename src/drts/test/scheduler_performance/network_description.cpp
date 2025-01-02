// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <network_description.hpp>

#include <boost/optional.hpp>

#include <fmt/core.h>

namespace drts
{
  namespace test
  {
    std::string create_network_description
      ( std::string const& name
      , boost::optional<std::string> const& with_num_worker_prop
      , boost::optional<std::string> const& with_memory_buffers
      )
    {
      return fmt::format (R"EOS(
            <defun name="{0}">
              <in name="num_tasks" type="long" place="num_tasks"/>
              <out name="done" type="control" place="done"/>

              <net>
                <place name="num_tasks" type="long"/>
                <place name="all_finished" type="control"/>
                <place name="done" type="control"/>

                <include-template href="triple.xml"/>
                <specialize name="triple_task_number" use="triple">
                  <type-map replace="T" with="long"/>
                </specialize>

                <place name="total_num_tasks" type="long"/>
                <place name="num_tasks_gen" type="long"/>
                <place name="num_remaining_tasks" type="long"/>
                <transition name="triple_num_tasks" inline="true">
                  <use name="triple_task_number"/>
                  <connect-in port="in" place="num_tasks"/>
                  <connect-out port="one" place="total_num_tasks"/>
                  <connect-out port="two" place="num_tasks_gen"/>
                  <connect-out port="three" place="num_remaining_tasks"/>
                </transition>

                <place name="task_id_A" type="long"/>
                <place name="task_id_B" type="long"/>

                <transition name="generate_task_ids_A" inline="true">
                  <defun>
                    <inout name="amount" type="long"/>
                    <in name="total_num_tasks" type="long"/>
                    <out name="task_id_A" type="long"/>
                    <expression>
                      ${{task_id_A}} := ${{amount}};
                      ${{amount}} := ${{amount}} - 1L;
                    </expression>
                    <condition>
                      (${{amount}} :gt: (${{total_num_tasks}} div 2L))
                    </condition>
                  </defun>
                  <connect-inout port="amount" place="num_tasks_gen"/>
                  <connect-read port="total_num_tasks" place="total_num_tasks"/>
                  <connect-out port="task_id_A" place="task_id_A"/>
                </transition>

                <transition name="generate_task_ids_B" inline="true">
                 <defun>
                    <inout name="amount" type="long"/>
                    <in name="total_num_tasks" type="long"/>
                    <out name="task_id_B" type="long"/>
                    <expression>
                      ${{task_id_B}} := ${{amount}};
                      ${{amount}} := ${{amount}} - 1L;
                    </expression>
                    <condition>
                      (${{amount}} :le: (${{total_num_tasks}} div 2L)) :and: (${{amount}} :gt: 0L)
                    </condition>
                  </defun>
                  <connect-inout port="amount" place="num_tasks_gen"/>
                  <connect-read port="total_num_tasks" place="total_num_tasks"/>
                  <connect-out port="task_id_B" place="task_id_B"/>
                </transition>

                <place name="task_finished" type="control"/>

                <transition name="compute_A">
                  <defun>
                    {1}
                    <require key="A"/>
                    <in name="task_id_A" type="long"/>
                    <out name="task_finished" type="control"/>
                    {2}
                    <module name="module_{0}"
                            function="task_A (task_finished{3})">
                      <code><![CDATA[
                        {4}
                        task_finished = we::type::literal::control();
                      ]]></code>
                    </module>
                  </defun>
                  <connect-in port="task_id_A" place="task_id_A"/>
                  <connect-out port="task_finished" place="task_finished"/>
                </transition>

                <transition name="compute_B">
                  {1}
                  <defun>
                    <require key="B"/>
                    <in name="task_id_B" type="long"/>
                    <out name="task_finished" type="control"/>
                    {2}
                    <module name="module_{0}"
                            function="task_B (task_finished{3})">
                      <code><![CDATA[
                        {4}
                        task_finished = we::type::literal::control();
                      ]]></code>
                    </module>
                  </defun>
                  <connect-in port="task_id_B" place="task_id_B"/>
                  <connect-out port="task_finished" place="task_finished"/>
                </transition>

                <include-template href="wait.xml"/>
                <specialize name="wait_control" use="wait">
                  <type-map replace="T" with="control"/>
                </specialize>

                <transition name="wait">
                  <use name="wait_control"/>
                  <place-map virtual="wait" real="num_remaining_tasks"/>
                  <connect-in port="trigger" place="task_finished"/>
                  <connect-out port="done" place="all_finished"/>
                </transition>

                <transition name="cleanup" inline="true">
                  <defun>
                    <in name="total_num_tasks" type="long"/>
                    <in name="num_tasks_gen" type="long"/>
                    <in name="all_finished" type="control"/>
                    <out name="done" type="control"/>
                    <expression>
                      ${{done}} := [];
                    </expression>
                  </defun>
                  <connect-in port="total_num_tasks" place="total_num_tasks"/>
                  <connect-in port="num_tasks_gen" place="num_tasks_gen"/>
                  <connect-in port="all_finished" place="all_finished"/>
                  <connect-out port="done" place="done"/>
                </transition>
              </net>
            </defun>)EOS"
        , name
        , with_num_worker_prop.get_value_or ("")
        , with_memory_buffers.get_value_or ("")
        , (!!with_memory_buffers ? ",local" : "")
        , (!!with_memory_buffers ? "(void) local;" : "")
        );
    }
  }
}
