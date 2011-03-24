/*
 * =====================================================================================
 *
 *       Filename:  ApplicationGuiEvent.hpp
 *
 *    Description:  applicatiin gui notification event
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_APP_GUI_EVENT_HPP
#define SDPA_APP_GUI_EVENT_HPP 1

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>

namespace sdpa { namespace daemon {
  class ApplicationGuiEvent
  {
  public:


    ApplicationGuiEvent(){}

    ApplicationGuiEvent(const int& row, const int& col, const std::string& result)
    	: row_(row)
      	, col_(col)
      	, result_(result)
    {}

    const int& row() const   { return row_; }
    int& row() { return row_; }

    const int& col() const   { return col_; }
    int& col() { return col_; }

    const std::string& result() const { return result_; }
    std::string& result() { return result_; }

  private:
    int row_;
    int col_;
    std::string result_;
  };
}}

namespace boost { namespace serialization {
  template <class Archive>
  void serialize(Archive &ar, sdpa::daemon::ApplicationGuiEvent &e, const unsigned int)
  {
    ar & e.row();
    ar & e.col();
    ar & e.result();
  }
}}

#endif
