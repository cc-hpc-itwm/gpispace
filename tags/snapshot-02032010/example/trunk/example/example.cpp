/***********************************************************************/
/** @file example.cpp
 *
 *
 *  @author Kai Krueger
 *  @date   2009-05-13
 *
 * (C) Fraunhofer ITWM Kaiserslautern
 **/
/*---------------------------------------------------------------------*/

#include "SdpaExampleFSMHandler.hpp"

/**
 Beschreibung einer StateMachine

 @dotfile example_sm.dot

@msc
    Sender,Receiver;
    Sender->Receiver [label="Command()", URL="\ref Receiver::Command()"];
    Sender<-Receiver [label="Ack()", URL="\ref Ack()", ID="1"];
 @endmsc

@msc
  hscale = "1";

  a,b,c;

  a->b [ label = "ab()" ] ;
  b->c [ label = "bc(TRUE)"];
  c=>c [ label = "process(1)" ];
  c=>c [ label = "process(2)" ];
  ...;
  c=>c [ label = "process(n)" ];
  c=>c [ label = "process(END)" ];
  a<<=c [ label = "callback()"];
  ---  [ label = "If more to run", ID="*" ];
  a->a [ label = "next()"];
  a->c [ label = "ac1()\nac2()"];
  b<-c [ label = "cb(TRUE)"];
  b->b [ label = "stalled(...)"];
  a<-b [ label = "ab() = FALSE"];
@endmsc

 */
namespace sdpa {
  namespace example {
/** 
 * @brief Nur ein Konstruktor
**/
    SdpaExampleFSMHandler::SdpaExampleFSMHandler ()
    {
      
    }

/*---------------------------------------------------------------------*/
/** 
 * @brief Kurze Beschreibung von grantLicenseRequest

 @param[in] event   ist eine Eingangsvariable

 @retval true bei Fehler
 @retval false wenn kein Fehler
**/
/*---------------------------------------------------------------------*/
    bool SdpaExampleFSMHandler::grantLicenseRequest (
      const gl::messages::LicenseRequestEvent& event
      )
    {
      return true;
    }
    

  } /* namespace example */;
}; /* namespace sdpa */

