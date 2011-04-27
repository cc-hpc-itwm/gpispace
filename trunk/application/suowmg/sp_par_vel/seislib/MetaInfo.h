//
// C++ Interface: MetaInfoBase
//
// Description: 
//
//
// Author: Daniel Gruenewald, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef METAINFO
#define METAINFO

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/// Class to maintain the data meta information
/// for a dataset of type typename

template <typename MetaInfoType, typename DataType>
class MetaInfoBase {
  
  public:
    
    typedef DataType data_type;
    
    MetaInfoBase();
    MetaInfoBase(const char *);
    MetaInfoBase(const MetaInfoBase &);
    MetaInfoBase(MetaInfoBase &);
    ~MetaInfoBase();
    
    MetaInfoBase & operator=(const MetaInfoBase &);
    
    // Arithmetic operators:
    // Need to be implemented by each of the 
    // derived classes
    
    // addition operator
    MetaInfoBase operator+(const MetaInfoBase &) const;
    
    // subtraction operator
    MetaInfoBase operator-(const MetaInfoBase &) const;
    
    // multiplication operator
    MetaInfoBase operator*(const MetaInfoBase &) const;
    
    // scalar multiplication operator
    MetaInfoBase operator*(const DataType &) const;
    
    /// equality operator
    bool operator==(const MetaInfoBase &) const;

	/// unequality operator
	bool operator!=(const MetaInfoBase &) const;

    // get routines:
    const MetaInfoType * getMetaInfo() const;
          MetaInfoType * getMetaInfo();
    
    static size_t getSize();
    
    // set routines:
    void setMetaInfo(char * const);

    // << operator
    template <typename MetaInfoType_OS, typename DataType_OS>
      friend std::ostream& operator<<( std::ostream & ,const MetaInfoBase<MetaInfoType_OS,DataType_OS> &);
  
  private:
    
  // member variables  
  private:
    
    // pointer to the meta information
    MetaInfoType * pMetaInfo;
    
    // boolean to indicate whether the current instance 
    // has allocated memory itself or whether it has been
    // given some memory
    bool selfalloc;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//                                                         //
//                MetaInfoBase Implementation                  //
//                                                         //
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

template <typename MetaInfoType , typename DataType>
    MetaInfoBase<MetaInfoType,DataType>::MetaInfoBase(const char * _pMetaInfo)
  : pMetaInfo((MetaInfoType *)_pMetaInfo),selfalloc(false)
{
  
}

template <typename MetaInfoType , typename DataType>
    MetaInfoBase<MetaInfoType,DataType>::MetaInfoBase()
  : selfalloc(true)
{
  pMetaInfo = (MetaInfoType *) new MetaInfoType;
}

template <typename MetaInfoType , typename DataType>
    MetaInfoBase<MetaInfoType,DataType>::MetaInfoBase(const MetaInfoBase<MetaInfoType,DataType> & rhs)
  : selfalloc(true)
{
  pMetaInfo = (MetaInfoType *) new MetaInfoType;
  
  memcpy((void *)pMetaInfo,(void *)rhs.pMetaInfo,getSize());
}

template <typename MetaInfoType , typename DataType>
    MetaInfoBase<MetaInfoType,DataType>::~MetaInfoBase()
{
  if(selfalloc)
    delete pMetaInfo;
}

template <typename MetaInfoType , typename DataType>
    MetaInfoBase<MetaInfoType,DataType> & MetaInfoBase<MetaInfoType,DataType>::operator=(const MetaInfoBase<MetaInfoType,DataType> & rhs)
{
  if( this != &rhs)
  {
    memcpy((void *)pMetaInfo,(void *)rhs.pMetaInfo,getSize());
  }
  
  return *this;
}

// addition operator
template <typename MetaInfoType , typename DataType>
    MetaInfoBase<MetaInfoType,DataType> MetaInfoBase<MetaInfoType,DataType>::operator+(const MetaInfoBase<MetaInfoType,DataType> & rhs) const
{
  MetaInfoBase<MetaInfoType,DataType> res;
  
  (*(res.pMetaInfo)) = (*pMetaInfo) + (*(rhs.pMetaInfo));
   
  return res;
  
}

// subtraction operator
template <typename MetaInfoType , typename DataType>
    MetaInfoBase<MetaInfoType,DataType> MetaInfoBase<MetaInfoType,DataType>::operator-(const MetaInfoBase<MetaInfoType,DataType> & rhs) const
{
  MetaInfoBase<MetaInfoType,DataType> res;
  
  (*(res.pMetaInfo)) = (*pMetaInfo) - (*(rhs.pMetaInfo));
   
  return res;
}

// multiplication operator
template <typename MetaInfoType , typename DataType>
    MetaInfoBase<MetaInfoType,DataType> MetaInfoBase<MetaInfoType,DataType>::operator*(const MetaInfoBase<MetaInfoType,DataType> & rhs) const
{
  MetaInfoBase<MetaInfoType,DataType> res;
  
  (*(res.pMetaInfo)) = (*pMetaInfo) * (*(rhs.pMetaInfo));
   
  return res;
}

// scalar multiplication operation
template <typename MetaInfoType , typename DataType>
    MetaInfoBase<MetaInfoType,DataType> MetaInfoBase<MetaInfoType,DataType>::operator*(const DataType & rhs) const
{
  MetaInfoBase<MetaInfoType,DataType> res;
  
  (*(res.pMetaInfo)) = (*pMetaInfo) * rhs;
   
  return res;
}

// equality operator
template <typename MetaInfoType , typename DataType>
    bool MetaInfoBase<MetaInfoType,DataType>::operator==(const MetaInfoBase<MetaInfoType,DataType> & rhs) const
{
  return ( *pMetaInfo == *(rhs.pMetaInfo) );
}

// unequality operator
template <typename MetaInfoType , typename DataType>
    bool MetaInfoBase<MetaInfoType,DataType>::operator!=(const MetaInfoBase<MetaInfoType,DataType> & rhs) const
{
  return ( !( *this == rhs ) );
}

template <typename MetaInfoType , typename DataType>
    const MetaInfoType * MetaInfoBase<MetaInfoType,DataType>::getMetaInfo() const
{ 
  return pMetaInfo;
}

template <typename MetaInfoType , typename DataType>
MetaInfoType * MetaInfoBase<MetaInfoType,DataType>::getMetaInfo()
{ 
  return pMetaInfo;
}

template <typename MetaInfoType , typename DataType>
void MetaInfoBase<MetaInfoType,DataType>::setMetaInfo(char * const _pMetaInfo) 
{ 
  selfalloc = false;
  pMetaInfo = (MetaInfoType *)_pMetaInfo;
}

template <typename MetaInfoType , typename DataType>
    size_t MetaInfoBase<MetaInfoType,DataType>::getSize()
{ 
  return sizeof(MetaInfoType);
}

template <typename MetaInfoType, typename DataType>
  std::ostream& operator<<( std::ostream & os,const MetaInfoBase<MetaInfoType,DataType> & MetaInfo)
{
  MetaInfoType out = *(MetaInfo.getMetaInfo());
  os<<"MetaInfoBase = "<< out;
  
  return os;
}

#endif
