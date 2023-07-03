// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/layer.hpp>
#include <we/type/Activity.hpp>

#include <we/test/layer.common.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <functional>
#include <list>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <tuple>

namespace
{
  void submit_fake ( std::vector<we::layer::id_type>* ids
                   , we::layer::id_type id
                   , we::type::Activity
                   )
  {
    ids->push_back (id);
  }

  void finished_fake ( volatile bool* finished
                     , we::layer::id_type
                     , we::type::Activity
                     )
  {
    *finished = true;
  }

  void cancel (we::layer::id_type){}
  void failed (we::layer::id_type, std::string){}
  void canceled (we::layer::id_type){}
  void token_put (std::string, ::boost::optional<std::exception_ptr>){}
  void workflow_response_response (std::string, ::boost::variant<std::exception_ptr, pnet::type::value::value_type>){}

  std::mutex generate_id_mutex;
  we::layer::id_type generate_id()
  {
    std::unique_lock<std::mutex> const _ (generate_id_mutex);
    static unsigned long _cnt (0);
    return std::to_string (++_cnt);
  }
}

BOOST_AUTO_TEST_CASE
  (performance_finished_shall_be_called_after_finished_N_childs)
{
  const std::size_t num_activities (10);
  const std::size_t num_child_per_activity (250);

  we::type::Activity activity_input;
  we::type::Activity activity_output;
  we::type::Activity activity_child;
  we::type::Activity activity_result;
  std::tie (activity_input, activity_output, activity_child, activity_result)
    = activity_with_child (num_child_per_activity);

  std::vector<we::layer::id_type> child_ids;
  child_ids.reserve (num_child_per_activity * num_activities);

  bool finished (false);

  std::mt19937 _random_engine;

  we::layer layer
    ( std::bind (&submit_fake, &child_ids, std::placeholders::_1, std::placeholders::_2)
    , std::bind (&cancel, std::placeholders::_1)
    , std::bind (&finished_fake, &finished, std::placeholders::_1, std::placeholders::_2)
    , &failed
    , &canceled
    , &token_put
    , &workflow_response_response
    , &generate_id
    , _random_engine
    );

  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (1))
  {
    for (std::size_t i (0); i < num_activities; ++i)
    {
      layer.submit (generate_id(), activity_input);
    }

    //! \todo Don't busy wait
    while (child_ids.size() != child_ids.capacity())
    {
      std::this_thread::yield();
    }

    for (we::layer::id_type child_id : child_ids)
    {
      layer.finished (child_id, activity_result);
    }

    //! \todo Don't busy wait
    while (!finished)
    {
      std::this_thread::yield();
    }
  };
}
