/***********************************************************************/
/** @file asianPG_PIPE.h
 *
 * $Id:$
 *
 * <short description>
 * <long description>
 *
 *  @author Kai Krueger
 *  @date   2009-01-06
 *  @email  krueger@itwm.fraunhofer.de
 *
 * (C) Fraunhofer ITWM Kaiserslautern
 **/
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * System headers
 *
 *---------------------------------------------------------------------*/
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>

/*---------------------------------------------------------------------*
 * Local headers
 *
 *---------------------------------------------------------------------*/
#include <gl/gl-common.hpp>
#include <gl/ClientObserver.hpp>
#include "asian.h"

/*---------------------------------------------------------------------*
 * Macros
 *
 *---------------------------------------------------------------------*/
namespace fs = boost::filesystem;

/*---------------------------------------------------------------------*
 * Structures, unions, enums an d typedefs
 *
 *---------------------------------------------------------------------*/
class asianAPI {
  public:
  asianAPI(gl::ClientObserver::Ptr pClientObserver, fs::ifstream &ifs );
  ~asianAPI();

  void info();
  int  prepare();
  void run();
  void post(std::string &sResult);

  private:
  int convertstring(const char *m_szParamBuffer);

  private:
  GL_DECLARE_LOGGER();
  gl::ClientObserver::Ptr m_pClientObserver;
  const char* m_szParamBuffer;

    param_t  m_cParam;
    result_t m_stResult;
    
};

/*---------------------------------------------------------------------*
 * File scope Variables (Variables share by several functions in
 *                       the same file )
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * External Variables
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Extern Functions declarations
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Functions declarations
 *
 *---------------------------------------------------------------------*/

