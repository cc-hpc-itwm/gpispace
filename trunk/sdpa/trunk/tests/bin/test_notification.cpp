/*
 * =====================================================================================
 *
 *       Filename:  test_notification.cpp
 *
 *    Description:  test the notification service
 *
 *        Version:  1.0
 *        Created:  11/19/2009 12:27:14 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/fhglog.hpp>
#include <sdpa/daemon/NotificationService.hpp>

using namespace sdpa::daemon;

int main(int ac, char **av)
{
  std::string gui_location("127.0.0.1:9000");
  if (ac > 1)
  {
    gui_location = av[1];
  }

  std::clog << "initializing notification service..." << std::endl;

  typedef NotificationService gui_service;
  gui_service notify_gui("SDPA", gui_location);
  for (size_t i = 0; i < 1000 ; ++i)
  {
    std::ostringstream ostr;
    ostr << "activity-" << i;
    const std::string aid(ostr.str());

    for (int j = gui_service::event_t::STATE_MIN; j <= gui_service::event_t::STATE_MAX; ++j)
    {
      notify_gui().activity_id(aid).activity_name("function call placeholder").activity_state(static_cast<gui_service::event_t::state_t>(j));
    }

    if ( (i % 100) == 0 ) sleep (1);
  }
}
