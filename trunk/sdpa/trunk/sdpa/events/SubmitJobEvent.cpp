/*
 * =====================================================================================
 *
 *       Filename:  SubmitJobEvent.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/28/2009 12:28:23 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "SubmitJobEvent.hpp"
#include <sdpa/events/sdpa-msg.pb.h>

namespace sdpa { namespace events {
  std::string SubmitJobEvent::encode() const
  {
    sdpa::events::detail::SDPAMessage msg;
    msg.set_message_id("FIXME: message-id");
    sdpa::events::detail::JobMessage *job_msg = msg.mutable_job_message();
    job_msg->set_job_id(job_id());
    sdpa::events::detail::SubmitJob *submit_job = job_msg->mutable_submit_job();
    submit_job->set_workflow(description());
    submit_job->set_parent_id(parent_id());

    std::string s;
    msg.SerializeToString(&s);
    return s;
  }
}}
