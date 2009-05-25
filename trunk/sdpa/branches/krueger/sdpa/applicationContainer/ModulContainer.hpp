/***********************************************************************/
/** @file ModulContainer.hpp
 *
 * $Id:$
 *
 * <short description>
 * <long description>
 *
 *  @author Kai Krueger
 *  @date   2009-05-20
 *  @email  kai.krueger@itwm.fhg.de
 *
 * (C) Fraunhofer ITWM Kaiserslautern
 **/
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * System headers
 *
 *---------------------------------------------------------------------*/
#include <string>
#include <map>
#include <vector>

/*---------------------------------------------------------------------*
 * Local headers
 *
 *---------------------------------------------------------------------*/
#include <sdpa/memory.hpp>
#include "FunctionContainer.hpp"

/*---------------------------------------------------------------------*
 * Macros
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Structures, unions, enums an d typedefs
 *
 *---------------------------------------------------------------------*/
namespace sdpa {
  namespace appcontainer {

    struct ltstr {
      bool operator()(const std::string s1, const std::string s2) const {
        return s1 < s2;
      }
    };

    class ModulContainer;

    typedef int (*fktn2_t)(ModulContainer*);
    typedef std::map<const std::string, FunctionContainer, ltstr>  MAP_FKTN_T;

    class ModulContainer {
    public:
        typedef sdpa::shared_ptr<ModulContainer> Ptr;

      ModulContainer(std::string sFilename);
      ~ModulContainer();
      
    private:
      void *_dynl_open();
      void *_dynl_sym(const char *sSymbol);
      int _dynl_close();
      
      
    private:
      std::string m_sFilename;              /**< name of shared object */
      void *m_pHandle;                           /**< modul handler (to shared object) */
      fktn2_t *m_pFuncInit;                       /**< pointer to initialisation function */
      MAP_FKTN_T m_Function_Map; /**< map of valid functions, filled by pFuncInit() */
      //std::vector<FunctionContainer> m_pFunctionList;

    };
  }
}

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

