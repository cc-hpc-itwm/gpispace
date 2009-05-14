/***********************************************************************/
/** @file SdpaExampleFSMHandler.hpp
 *
 *
 *  @author Kai Krueger
 *  @date   2009-05-13
 *
 * (C) Fraunhofer ITWM Kaiserslautern
 **/
/*---------------------------------------------------------------------*/

#ifndef _SdpaExampleFSMHandler_hpp_
#define _SdpaExampleFSMHandler_hpp_

#include <string.h>

namespace gl {
  namespace messages {
    class  LicenseRequestEvent 
    {};
    class ReceivedKeyEvent
    {};
    class ReceivedNakEvent
    {};
    class RequestKeyEvent
    {};
    class ClosedEvent
    {};
    
  }
}


namespace sdpa {
  namespace example {
    class SdpaExampleFSMHandler {

    public:
      SdpaExampleFSMHandler();
      virtual bool grantLicenseRequest (const gl::messages::LicenseRequestEvent&)=0;
      virtual void handleIssueLicense(const gl::messages::LicenseRequestEvent&)=0;
      virtual void handleLicenseNak(const gl::messages::LicenseRequestEvent&)=0;
      virtual void handleReceivedNak(const gl::messages::ReceivedNakEvent&)=0;
      virtual void handleRequestKey(const gl::messages::RequestKeyEvent&)=0;
      virtual void handleReceivedKey(const gl::messages::ReceivedKeyEvent&)=0;
      virtual void handleClosed(const gl::messages::ClosedEvent&)=0;

    private:
      int m_nZahl1; /**< Beschreibung von Zahl1 hier */
      /** man kann Zahl2 auch so beschreiben */
      int m_nZahl2;
    };
    
  };
};

#endif /* _SdpaExampleFSMHandler_hpp_ */
