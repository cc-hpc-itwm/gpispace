/*
 * =====================================================================================
 *
 *       Filename:  test_codec.cpp
 *
 *    Description:  tests the encoding/decoding of events
 *
 *        Version:  1.0
 *        Created:  10/30/2009 01:43:03 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <iostream>

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>
#include <sdpa/events/Codec.hpp>

int main(int, char **)
{
  using namespace sdpa::events;
  fhg::log::Configurator::configure();

  int errcount(0);

  Codec codec;
  {
    std::clog << "testing SubmitJobEvent...";
    SubmitJobEvent e("foo", "bar", "job-id-1", "<desc></desc>", "parent-job-id-0");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (SubmitJobEvent *e2 = dynamic_cast<SubmitJobEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id()
       || e2->description() != e.description()
       || e2->parent_id() != e.parent_id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a SubmitJobEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing SubmitJobAckEvent...";
    SubmitJobAckEvent e("foo", "bar", "job-id-1", "42");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (SubmitJobAckEvent *e2 = dynamic_cast<SubmitJobAckEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id()
	   || e2->id() != e.id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a SubmitJobAckEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing CancelJobEvent...";
    CancelJobEvent e("foo", "bar", "job-id-1", "test reason");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (CancelJobEvent *e2 = dynamic_cast<CancelJobEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a CancelJobEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing CancelJobAckEvent...";
    CancelJobAckEvent e("foo", "bar", "job-id-1");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (CancelJobAckEvent *e2 = dynamic_cast<CancelJobAckEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id()
	   || e2->id() != e.id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a CancelJobAckEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing DeleteJobEvent...";
    DeleteJobEvent e("foo", "bar", "job-id-1");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (DeleteJobEvent *e2 = dynamic_cast<DeleteJobEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a DeleteJobEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing DeleteJobAckEvent...";
    DeleteJobAckEvent e("foo", "bar", "job-id-1", "42");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (DeleteJobAckEvent *e2 = dynamic_cast<DeleteJobAckEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id()
	   || e2->id() != e.id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a DeleteJobAckEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing ErrorEvent...";
    ErrorEvent e("foo", "bar", ErrorEvent::SDPA_EBUSY, "busy");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (ErrorEvent *e2 = dynamic_cast<ErrorEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->reason() != e.reason()
       || e2->error_code() != e.error_code()
      )
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a ErrorEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing JobFailedEvent...";
    sdpa::job_result_t result;
    JobFailedEvent e("foo", "bar", "job-id-1", result);
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (JobFailedEvent *e2 = dynamic_cast<JobFailedEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a JobFailedEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing JobFailedAckEvent...";
    JobFailedAckEvent e("foo", "bar", "job-id-1");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (JobFailedAckEvent *e2 = dynamic_cast<JobFailedAckEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id()
	   || e2->id() != e.id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a JobFailedAckEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing JobFinishedEvent...";
    sdpa::job_result_t result;
    JobFinishedEvent e("foo", "bar", "job-id-1", result);
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (JobFinishedEvent *e2 = dynamic_cast<JobFinishedEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a JobFinishedEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing JobFinishedAckEvent...";
    JobFinishedAckEvent e("foo", "bar", "job-id-1");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (JobFinishedAckEvent *e2 = dynamic_cast<JobFinishedAckEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id()
	   || e2->id() != e.id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a JobFinishedAckEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing JobResultsReplyEvent...";
    sdpa::job_result_t result;
    JobResultsReplyEvent e("foo", "bar", "job-id-1", result);
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (JobResultsReplyEvent *e2 = dynamic_cast<JobResultsReplyEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id()
      )
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a JobResultsReplyEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing JobStatusReplyEvent...";
    JobStatusReplyEvent e("foo", "bar", "job-id-1");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (JobStatusReplyEvent *e2 = dynamic_cast<JobStatusReplyEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a JobStatusReplyEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing QueryJobStatusEvent...";
    QueryJobStatusEvent e("foo", "bar", "job-id-1");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (QueryJobStatusEvent *e2 = dynamic_cast<QueryJobStatusEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a QueryJobStatusEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing RetrieveJobResultsEvent...";
    RetrieveJobResultsEvent e("foo", "bar", "job-id-1");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (RetrieveJobResultsEvent *e2 = dynamic_cast<RetrieveJobResultsEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->job_id() != e.job_id())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a RetrieveJobResultsEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing WorkerRegistrationEvent...";
    const sdpa::events::SDPAEvent::address_t from ("from");
    const sdpa::events::SDPAEvent::address_t to ("to");
    const sdpa::worker_id_t worker_id ("worker_id");

    const unsigned int capacity (10);
    WorkerRegistrationEvent e(from, to, capacity);
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (WorkerRegistrationEvent *e2 = dynamic_cast<WorkerRegistrationEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a WorkerRegistrationEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing WorkerRegistrationAckEvent...";
    WorkerRegistrationAckEvent e("foo", "bar");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (WorkerRegistrationAckEvent *e2 = dynamic_cast<WorkerRegistrationAckEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to())
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a WorkerRegistrationAckEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing LifeSignEvent...";
    LifeSignEvent e("foo", "bar", "last-job-0");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (LifeSignEvent *e2 = dynamic_cast<LifeSignEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->last_job_id() != e.last_job_id()
      )
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a LifeSignEvent!" << std::endl;
    }
  }

  {
    std::clog << "testing RequestJobEvent...";
    RequestJobEvent e("foo", "bar", "last-job-0");
    const std::string encoded = codec.encode(&e);
    SDPAEvent *d = codec.decode(encoded);

    if (RequestJobEvent *e2 = dynamic_cast<RequestJobEvent*>(d))
    {
      if (e2->from() != e.from()
       || e2->to() != e.to()
       || e2->last_job_id() != e.last_job_id()
      )
      {
        ++errcount;
        std::clog << "FAILED!" << std::endl;
        std::clog << "\tone or more data fields don't match!" << std::endl;
      }
      else
      {
        std::clog << "OK!" << std::endl;
      }
    }
    else
    {
      ++errcount;
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tdecoded event is not a RequestJobEvent!" << std::endl;
    }
  }

  return errcount;
}
