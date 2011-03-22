/***********************************************************************/
/** @file asianPG_PIPE.cpp
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
#include <exception>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

/*---------------------------------------------------------------------*
 * Local headers
 *
 *---------------------------------------------------------------------*/
#include "asianAPI.h"

/*---------------------------------------------------------------------*
 * Macros
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Structures, unions, enums an d typedefs
 *
 *---------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------*/
/** 
 * @brief contructor

 @param[in] argv    commandline arguments
 @param[in] argc    commandline argument counter
**/
/*---------------------------------------------------------------------*/
asianAPI::asianAPI(gl::ClientObserver::Ptr pClientObserver, fs::ifstream &ifs) :
    GL_INIT_LOGGER("gl-asianAPI"),
    m_pClientObserver(pClientObserver)
{
  
  while(!ifs.eof()) {
    std::string s;
    std::getline(ifs,s, '\n');
    //std::cout<<"@"<<s<<"@"<<std::endl;
    convertstring(s.c_str());
  }
  
}

asianAPI::~asianAPI() {
}

void asianAPI::info() {
  std::ostringstream oss;
  oss<<"S     = "<<m_cParam.m_dS<<std::endl;
  oss<<"K     = "<<m_cParam.m_dK<<std::endl;
  oss<<"T     = "<<m_cParam.m_dT<<std::endl;
  oss<<"Sigma = "<<m_cParam.m_dSigma<<std::endl;
  oss<<"r     = "<<m_cParam.m_dr<<std::endl;
  oss<<"d     = "<<m_cParam.m_dd<<std::endl;
  oss<<"1.Fixing= "<<m_cParam.m_nFirstFixing<<std::endl;
  oss<<"Fixing/Jahr = "<<m_cParam.m_dFixingsProJahr<<std::endl;
  oss<<"# Dividende = "<<m_cParam.m_nAnzahlderDividende<<std::endl;
  oss<<"n     = "<<m_cParam.m_nn<<std::endl;
  oss<<"d1    = "<<m_cParam.m_dD1<<std::endl;
  oss<<"d2    = "<<m_cParam.m_dD2<<std::endl;
  m_pClientObserver->gl_update_state(gl::HAS_LICENSE, oss.str());
  GL_LOG_INFO(oss.str());
}

int asianAPI::prepare() {
  //fs::ifstream ifs
  //fprintf(stderr, "prepare c=%d\n", m_cArrInItems.GetSize());
  //convertstring();
  //fprintf(stderr, "readpipe done\n");
  
  return 0;
}

void asianAPI::run() {

  double dErgebnis[4];
  double dStdDev[4];
  double dS[4];
  double dSigma[4];

  dS[0]     = m_cParam.m_dS;
  dSigma[0] = m_cParam.m_dSigma;

  dS[1]     = m_cParam.m_dS + m_cParam.m_dD1;
  dSigma[1] = m_cParam.m_dSigma;
  
  dS[2]     = m_cParam.m_dS + 2*m_cParam.m_dD1;
  dSigma[2] = m_cParam.m_dSigma;

  dS[3]     = m_cParam.m_dS;
  dSigma[3] = m_cParam.m_dSigma + m_cParam.m_dD2;

  info();
  mcinterface(m_cParam, dErgebnis[0], dStdDev[0]);

  m_cParam.m_dS     = dS[1];
  m_cParam.m_dSigma = dSigma[1];
  GL_LOG_INFO("Compute 1 of 3");
  mcinterface(m_cParam, dErgebnis[1], dStdDev[1]);

  m_cParam.m_dS     = dS[2];
  m_cParam.m_dSigma = dSigma[2];
  GL_LOG_INFO("Compute 2 of 3");
  mcinterface(m_cParam, dErgebnis[2], dStdDev[2]);

  m_cParam.m_dS     = dS[3];
  m_cParam.m_dSigma = dSigma[3];
  GL_LOG_INFO("Compute 3 of 3");
  mcinterface(m_cParam, dErgebnis[3], dStdDev[3]);

  // compute final results
  m_stResult.m_dSum1 = dErgebnis[0];
  m_stResult.m_dSum2 = dStdDev[0];
  m_stResult.m_dDelta = (dErgebnis[1] - dErgebnis[0])/(m_cParam.m_dD1);
  m_stResult.m_dGamma = (dErgebnis[2] - 2*dErgebnis[1] + dErgebnis[0])/
    (m_cParam.m_dD1 * m_cParam.m_dD1);
  m_stResult.m_dVega  = (dErgebnis[3] - dErgebnis[0]) / (m_cParam.m_dD2);
  
  std::ostringstream oss;
  oss<<"Sum1 = "<<m_stResult.m_dSum1<<std::endl;
  oss<<"Sum2 = "<<m_stResult.m_dSum2<<std::endl;
  oss<<"Delta = "<<m_stResult.m_dDelta<<std::endl;
  oss<<"Gamma = "<<m_stResult.m_dGamma<<std::endl;
  oss<<"Vega = "<<m_stResult.m_dVega<<std::endl;
  GL_LOG_INFO(oss.str());
}

void asianAPI::post(std::string &sResult) {
  //fprintf(stderr, "Asian PG STD post: \n");
  std::ostringstream oss;
  oss<<"Sum1 = "<<m_stResult.m_dSum1<<std::endl;
  oss<<"Sum2 = "<<m_stResult.m_dSum2<<std::endl;
  oss<<"Delta = "<<m_stResult.m_dDelta<<std::endl;
  oss<<"Gamma = "<<m_stResult.m_dGamma<<std::endl;
  oss<<"Vega = "<<m_stResult.m_dVega<<std::endl;
  m_pClientObserver->gl_update_state(gl::HAS_LICENSE, oss.str());
  sResult = oss.str();
}

int asianAPI::convertstring(const char *m_szParamBuffer) {
  
  // Konfigurationsdatei einlesen
  char var[512];
  
  float i;

  const char *p = m_szParamBuffer;
  //const char *q = m_szParamBuffer;
  //do {
  //p = q;
  memset(var, 0, sizeof(var));
  sscanf (p,"%s = %f",var,&i);
  //fprintf(stderr, "%s -> %f\n",var,i);

      if (!strcmp (var,"S")) m_cParam.m_dS = (double) i;
      else if (!strcmp (var,"K")) m_cParam.m_dK = (double) i;
      else if (!strcmp (var,"T")) m_cParam.m_dT = (double) i;
      else if (!strcmp (var,"sigma")) m_cParam.m_dSigma = (double) i;
      else if (!strcmp (var,"r")) m_cParam.m_dr = (double) i;
      else if (!strcmp (var,"d")) m_cParam.m_dd = (double) i;
      else if (!strcmp (var,"FirstFixing")) m_cParam.m_nFirstFixing = (int) i;
      else if (!strcmp (var,"FixingsProJahr"))  m_cParam.m_dFixingsProJahr= (int) i;
      else if (!strcmp (var,"AnzahlderDividende")) m_cParam.m_nAnzahlderDividende = (int) i;
      else if (!strcmp (var,"n")) m_cParam.m_nn = (long int) i;
      else if (!strcmp (var,"d1")) m_cParam.m_dD1 = (double) i;
      else if (!strcmp (var,"d2")) m_cParam.m_dD2 = (double) i;
      //    p++;
      //}
      //while ( (q = strchr(p, '\n')) != NULL && p!=NULL);
  
  return 0;
}
