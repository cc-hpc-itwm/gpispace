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

#include <sdpa/job_states.hpp>

#define TOSTR_(x) #x
#define TOSTR(x) TOSTR_(x)

#define CHECK_STATUS_TO_STR(s, txt)                                     \
  if (sdpa::status::show ((s)) != txt)                                  \
  {                                                                     \
    std::cerr << "E: status check for: " << TOSTR(s) << " failed" << std::endl; \
    std::cerr << "     expected: " << txt << std::endl;                 \
    std::cerr << "          got: " << sdpa::status::show ((s)) << std::endl; \
    ++errcount;                                                         \
  }

#define CHECK_STR_TO_STATUS(s, txt)                                     \
  if (sdpa::status::read ((txt)) != s)                                  \
  {                                                                     \
    std::cerr << "E: status check for: " << TOSTR(s) << " failed" << std::endl; \
    std::cerr << "     expected: " << s << std::endl;                 \
    std::cerr << "          got: " << sdpa::status::read ((txt)) << std::endl; \
    ++errcount;                                                         \
  }

int main(int, char **)
{
  int errcount;

  errcount = 0;

  CHECK_STATUS_TO_STR (sdpa::status::PENDING,  "SDPA::Pending");
  CHECK_STATUS_TO_STR (sdpa::status::SUSPENDED, "SDPA::Suspended");
  CHECK_STATUS_TO_STR (sdpa::status::RUNNING, "SDPA::Running");
  CHECK_STATUS_TO_STR (sdpa::status::FINISHED, "SDPA::Finished");
  CHECK_STATUS_TO_STR (sdpa::status::FAILED, "SDPA::Failed");
  CHECK_STATUS_TO_STR (sdpa::status::CANCELED, "SDPA::Canceled");
  CHECK_STATUS_TO_STR (sdpa::status::UNKNOWN, "SDPA::Unknown");
  CHECK_STATUS_TO_STR (42, "Strange job state");

  CHECK_STR_TO_STATUS (sdpa::status::PENDING,  "SDPA::Pending");
  CHECK_STR_TO_STATUS (sdpa::status::SUSPENDED, "SDPA::Suspended");
  CHECK_STR_TO_STATUS (sdpa::status::RUNNING, "SDPA::Running");
  CHECK_STR_TO_STATUS (sdpa::status::FINISHED, "SDPA::Finished");
  CHECK_STR_TO_STATUS (sdpa::status::FAILED, "SDPA::Failed");
  CHECK_STR_TO_STATUS (sdpa::status::CANCELED, "SDPA::Canceled");
  CHECK_STR_TO_STATUS (sdpa::status::UNKNOWN, "SDPA::Unknown");
  CHECK_STR_TO_STATUS (sdpa::status::UNKNOWN, "invalid state");

  return errcount;
}
