/*
 * =====================================================================================
 *
 *       Filename:  LocatorPersister.hpp
 *
 *    Description:  persist location information
 *
 *        Version:  1.0
 *        Created:  10/23/2009 11:55:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SEDA_COMM_LOCATOR_PERSISTER_HPP
#define SEDA_COMM_LOCATOR_PERSISTER_HPP 1

#include <seda/comm/exception.hpp>
#include <seda/comm/Locator.hpp>

namespace seda { namespace comm {
  class LocatorPersister
  {
  public:
    explicit
    LocatorPersister(const std::string &a_path)
      : path_(a_path)
    {}

    const std::string &path() const { return path_; }

    void store(const Locator &locator) const;
    void load(Locator &locator) const;
  private:
    std::string path_;
  };
}}

#endif
