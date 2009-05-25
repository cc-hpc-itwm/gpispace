/***********************************************************************/
/** @file FunctionContainer.hpp
 *
 * $Id:$
 *
 * <short description>
 * <long description>
 *
 *  @author Kai Krueger
 *  @date   2009-05-25
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
#include <vector>

/*---------------------------------------------------------------------*
 * Local headers
 *
 *---------------------------------------------------------------------*/
#include <sdpa/memory.hpp>
#include <sdpa/applicationContainer/Token.hpp>

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

    typedef void (*fktn_t)(std::vector<Token> &pIn, std::vector<Token> &pOut);

    class FunctionContainer {
    public:
        typedef sdpa::shared_ptr<FunctionContainer> Ptr;

      //FunctionContainer(std::string sFunctionName, fktn_t *pFunction) :
      FunctionContainer(std::string sFunctionName) ;
      ~FunctionContainer();
      
      
      //virtual void call(ArgumentContainer &pIn, ArgumentContainer &pOut)  = 0;
      virtual void call(std::vector<Token> &pIn, std::vector<Token> &pOut);
      
    private:
      std::string m_sFunctionName;
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

