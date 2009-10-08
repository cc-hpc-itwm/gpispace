
#ifndef XMLTranscode_H
#define XMLTranscode_H

#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_USE

// transcode with automatic release

namespace gwdl
{
    class StrXML
    {
    public :
      StrXML (const char* const toTranscode)
      {
        fLocalForm = XMLString::transcode(toTranscode);
      }

      ~StrXML()
      {
        XMLString::release(&fLocalForm);
      }

      XMLCh* utf16() const
      {
        return fLocalForm;
      }

private :
      XMLCh* fLocalForm;
    };

#define X(str) StrXML(str).utf16()
#define XS(strg) StrXML(strg.c_str()).utf16()
#define S(str) XMLString::transcode(str)
}

#endif
