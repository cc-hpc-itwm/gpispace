
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

#define X(str) XMLString::transcode((const char *)& str)
#define XS(strg) XMLString::transcode((const char*) strg.c_str())
#define S(str) XMLString::transcode(str)
}

#endif
