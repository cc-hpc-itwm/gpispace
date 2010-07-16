#include "XMLReader_red.h"
#include <stdio.h>



// helper to simplify parsing of arbitrary simple types
template<class T> bool convert( const string& s, T& t ) {
  istringstream in( s );
  in >> t;
  
  return !in.fail();
}

// helper to simplify parsing of arbitrary simple types
template<class T> bool genericGet( const XMLReader* r, const string& path, T& t ) {
  string tmp;
  
  if( r->getString( path, tmp ) )
    return convert( tmp, t );
  else
    return false;
}

template<> bool genericGet( const XMLReader* r, const string& path, string& t ) {
  return r->getString( path, t );
}

XMLReader::XMLReader() {
  xml = new CMarkupSTL();
}


XMLReader::~XMLReader() {
  delete xml;
}


bool XMLReader::setFile( const string& filename ) {
  if( xml->Load( filename.c_str() ) )
    {
      if( !xml->IsWellFormed() )
	{
	  printf( "XMLReader::setFile(): %s is not well formed XML!\n", filename.c_str() );
	  return false;
	}
      
      return true;
    }
  
  return false;
}

bool XMLReader::setString( const string& data ) {
  if( xml->SetDoc( data.c_str() ) ) {
      if( !xml->IsWellFormed() ) {
	printf( "XMLReader::setString(): document is not well formed XML!\n" );
	return false;
      }
      
      return true;
  }
  
  return false;
}

bool XMLReader::getString( const string& path, string& target ) const {
  xml->ResetPos();
  
  string::size_type n = path.rfind( '/' );
  
  if( path[n+1] == '@' ) {
    // last path element is an attribute
    string rpath = path.substr( 0, n );
    string rattr = path.substr( n+2, string::npos );
    
    if( traversePath( rpath ) ) {
      target = xml->GetAttrib( rattr.c_str() );
      
      // since CMarkup does return an empty string for
      // non-existent attributes, we count the empty string as
      // an error
      return !target.empty();
    }
  }
  else {
    // no attribute
    if(	traversePath( path ) ) {
      target = xml->GetData();
      return true;
    }
  }
  
  return false;
}

bool XMLReader::getChar( const string& path, char* target ) const {
  return genericGet( this, path, target );
}
bool XMLReader::getDouble( const string& path, double& target ) const {
  return genericGet( this, path, target );
}

bool XMLReader::getFloat( const string& path, float& target ) const {
  return genericGet( this, path, target );
}

bool XMLReader::getInt( const string& path, int& target ) const {
  return genericGet( this, path, target );
}

bool XMLReader::getShort( const string& path, short& target ) const {
  return genericGet( this, path, target );
}

bool XMLReader::getLong( const string& path, long& target ) const {
  return genericGet( this, path, target );
}

bool XMLReader::getItemsBelow( const string& path, 
			       vector<string>& target, const string& targetstring ) const {
  xml->ResetPos();
  
  
  bool result = traversePath( path );
  
  if( result ) {
    target.clear();
    
    while( xml->FindChildElem() ) {
      string childname = xml->GetChildTagName();
      bool accep=true;
      if ( path == "modules" ) {
	if ( childname != targetstring && targetstring != "all" )
	  accep = false;
      }
      
      if ( accep == true )
	target.push_back( childname );
    }
  }
  
  return result;
}

bool XMLReader::traversePath( const string& path ) const {
  string::size_type a=0, b=0;
  
  do {
    b = path.find( '/', a );
    string current = path.substr( a, b-a );
    
    if( xml->FindChildElem( current.c_str() ) )
      xml->IntoElem();
    else 
      return false;
    
    a = b + 1;
  }
  while( b != string::npos );
  
  return true;
}

string XMLReader::getSubDocument( const string& path ) const {
  xml->ResetPos();
  
  if( traversePath( path ) ) {
    int s, e;
    xml->GetOffsets( s, e );
    
    string tmp = xml->GetDoc().substr( s, e-s+1 );
    
    return tmp;
  }
  
  return string();
}

