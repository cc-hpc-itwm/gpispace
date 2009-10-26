/*
 * =====================================================================================
 *
 *       Filename:  exception.hpp
 *
 *    Description:  seda communication exceptions
 *
 *        Version:  1.0
 *        Created:  10/23/2009 10:24:18 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_EXCEPTION_HPP
#define SEDA_COMM_EXCEPTION_HPP 1

#include <stdexcept>

namespace seda { namespace comm {
  class CommunicationError : public std::exception
  {
  public:
    explicit
    CommunicationError(const std::string &a_reason)
      : std::exception()
      , reason_(a_reason)
    {}

    virtual ~CommunicationError() throw() {}

    const char *what() const throw ()
    {
      return reason_.c_str();
    }
  private:
    std::string reason_;
  };
}}

#endif
