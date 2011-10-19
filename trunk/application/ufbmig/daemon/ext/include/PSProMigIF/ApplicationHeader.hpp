/*! @brief This file the application enum
 *  The application enum holds informations about which application are available
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: May 05, 2010
 *  */

#ifndef APPLICATIONHEADER_HPP_
#define APPLICATIONHEADER_HPP_

#include <string>

namespace PSProMigIF
{
  /*! @brief The application enum holds informations about which application are available
   */
  enum Applications
  {
    Applications_IM = 0,
    Applications_SlantStack = 2,
    Applications_VisibilityAnalysis = 1,
#if SHOW_CURRENT_DEVELOPEMENT_FEATURES
    Applications_Applicationtest = 3,
#endif

#if defined ISIM_AT_SDPA_SUPPORT && SHOW_CURRENT_DEVELOPEMENT_FEATURES
    Applications_IMatSDPA = 4,
#else
    Applications_IMatSDPA = 3,
#endif

#if defined SHOW_CURRENT_DEVELOPEMENT_FEATURES && defined ISIM_AT_SDPA_SUPPORT
    Applications_Max = 5,
#elif defined SHOW_CURRENT_DEVELOPEMENT_FEATURES || defined ISIM_AT_SDPA_SUPPORT
    Applications_Max = 4,
#else
    Applications_Max = 3,
#endif

    Applications_ServerHelper1 = 100,
    Applications_ServerHelper2 = 101,
    Applications_ServerHelper3 = 102,
    Applications_ServerHelperMax = 103,

    Applications_Undefined = 200
  };

  /*! @return Returns a application name for the given application value
   */
  std::string ApplicationsName(const Applications& _application);

  std::string ApplicationsNameShort(const Applications& _application);

  /*! @return Converts an application name to an value of the application enum
   */
  Applications ApplicationsEnum(const std::string& _sApplication);

  Applications whichApplication(const std::string& _sString);
}

#endif
