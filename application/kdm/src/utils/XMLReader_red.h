#ifndef __xmlreader_h
#define __xmlreader_h

#include <string>
#include <vector>
#include "MarkupSTL_red.h"

#include <sstream>
#include <cstring>

#define TINF 1e6

using namespace std;


class CMarkupSTL;

class XMLReader {
  mutable CMarkupSTL* xml;
  
  bool traversePath( const std::string& path ) const;

 public:

  XMLReader();
  ~XMLReader();
  
  bool setFile( const std::string& filename );
  bool setString( const std::string& data );
  
  bool getString( const std::string& path, std::string& target ) const;
  bool getChar( const std::string& path, char* target ) const;
  bool getDouble( const std::string& path, double& target ) const;
  bool getFloat( const std::string& path, float& target ) const;
  bool getInt( const std::string& path, int& target ) const;
  bool getShort( const std::string& path, short& target ) const;
  bool getLong( const std::string& path, long& target ) const;

  bool getItemsBelow( const std::string& path, std::vector<std::string>& target, const std::string& ) const;
  
  std::string getSubDocument( const std::string& path ) const;
};

#endif // __xmlreader_h
