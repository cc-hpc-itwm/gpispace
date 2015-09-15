#define BOOST_TEST_MODULE layer_performance
#include <boost/test/unit_test.hpp>

#include <we/layer.hpp>
#include <we/type/activity.hpp>

#include <we/test/layer.common.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <functional>
#include <list>
#include <random>
#include <tuple>

namespace
{
  void submit_fake ( std::vector<we::layer::id_type>* ids
                   , we::layer::id_type id
                   , we::type::activity_t
                   )
  {
    ids->push_back (id);
  }

  void finished_fake ( volatile bool* finished
                     , we::layer::id_type
                     , we::type::activity_t
                     )
  {
    *finished = true;
  }

  void cancel (we::layer::id_type){}
  void failed (we::layer::id_type, std::string){}
  void canceled (we::layer::id_type){}
  void discover (we::layer::id_type, we::layer::id_type){}
  void discovered (we::layer::id_type, sdpa::discovery_info_t){}
  void token_put (std::string, boost::optional<std::exception_ptr>){}

  boost::mutex generate_id_mutex;
  we::layer::id_type generate_id()
  {
    boost::mutex::scoped_lock const _ (generate_id_mutex);
    static unsigned long _cnt (0);
    return boost::lexical_cast<we::layer::id_type> (++_cnt);
  }
}

BOOST_AUTO_TEST_CASE
  (performance_finished_shall_be_called_after_finished_N_childs)
{
  const std::size_t num_activities (10);
  const std::size_t num_child_per_activity (250);

  we::type::activity_t activity_input;
  we::type::activity_t activity_output;
  we::type::activity_t activity_child;
  we::type::activity_t activity_result;
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
    , &discover
    , &discovered
    , &token_put
    , &generate_id
    , _random_engine
    );

  fhg::util::testing::require_maximum_running_time<std::chrono::seconds>
    const maxmimum_running_time (1);

  for (std::size_t i (0); i < num_activities; ++i)
  {
    layer.submit (generate_id(), activity_input);
  }

  //! \todo Don't busy wait
  while (child_ids.size() != child_ids.capacity())
  {
    boost::this_thread::yield();
  }

  for (we::layer::id_type child_id : child_ids)
  {
    layer.finished (child_id, activity_result);
  }

  //! \todo Don't busy wait
  while (!finished)
  {
    boost::this_thread::yield();
  }
}
