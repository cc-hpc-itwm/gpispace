#ifndef POINTERMANAGER
#define POINTERMANAGER

/**
@author Daniel Gruenewald

\brief class implements a pointer class to NDimObj
\details
\pre
*/

enum Choice {NO,YES};

template <typename PointerType>
  class PointerHandle
{
  public:
    
    PointerHandle()
    : pObj(NULL)
     ,refCnt(0)
     ,destAtEnd(NO)
    {}
    
    PointerHandle(PointerType const _pObj, const Choice & _destAtEnd)
    : pObj(_pObj)
     ,refCnt(1)
     ,destAtEnd(_destAtEnd)
    {}
    
    ~PointerHandle()
    {
      if(destAtEnd == YES )
      {
	if( refCnt == 0)
	{
	  if( pObj != NULL)
	  {
	    delete pObj;
	  }
	}
      }      
    }
    
    long IncRefCnt()
    {
      return (++refCnt);
    }
    
    long DecRefCnt()
    {
      if ( (--refCnt) < 0 )
      {
	refCnt = 0;
      }
      return (refCnt);
    }
    
  public:
    
    PointerType pObj;
    long refCnt;
    Choice destAtEnd;
    
};

// call it EquivalenceObject_Ptr
template <typename PointerType, PointerType (*CopyObj)(PointerType)>
  class PointerManager
{
  public:
    
    // default constructor : OK
    PointerManager() 
    : pPointerHandle(NULL)
     ,original(YES)
    {}
    
    // constructor: OK
    PointerManager(PointerType const Pointer)
    : pPointerHandle(new PointerHandle<PointerType>(Pointer,NO) )
     ,original(YES)
    {}
    
    // copy constructor for const rhs: OK
    PointerManager(const PointerManager & rhs)
    : pPointerHandle(new PointerHandle<PointerType>((*CopyObj)(rhs.getRawPtr()),YES))
     ,original(YES)
    {
      //std::cout<<"Making a copy of the object referred to by the pointer"<<std::endl;
    }
    
    //assignment operator for const rhs: OK
    //const rhs means that the rhs represents a constant object
    PointerManager & operator=(const PointerManager & rhs)
    {
//       MakeObjModifyable();
//       // has to copy the things away such that the thing becomes modifyable withotu
//       // interacting the other routines.
//       
//       DeRef(pPointerHandle);
//       
//       pPointerHandle = new PointerHandle<PointerType>((*CopyObj)(rhs.getRawPtr()),YES);
//       
//       original = YES;
      if (pPointerHandle != rhs.pPointerHandle )
      {
	
	DeRef(pPointerHandle);
	//std::cout<<"Making a copy of the object referred to by the pointer"<<std::endl;
	pPointerHandle = new PointerHandle<PointerType>((*CopyObj)(rhs.getRawPtr()),YES);
	
	original = YES;
	
      }
      
      return *this;
      
    }
    
    // copy constructor for non constant rhs: OK
    PointerManager(PointerManager & rhs)
    : pPointerHandle(rhs.pPointerHandle)
     ,original(NO)
    {
      pPointerHandle->IncRefCnt();  
    }
    
    //assignment operator for non constant rhs: OK
    PointerManager & operator=(PointerManager & rhs)
    {
      if (pPointerHandle != rhs.pPointerHandle )
      {
	
	DeRef(pPointerHandle);
	
	pPointerHandle = rhs.pPointerHandle;
	
	pPointerHandle->IncRefCnt();
	
	original = NO;
	
      }
      
      return *this;
    }
    
    // return the raw pointer represented by the object: OK
    const PointerType getRawPtr() const
    {
      if (pPointerHandle != NULL)
      {
	return pPointerHandle->pObj;
      }
      else
      {
	return NULL;
      }
    }
    
  private:
    
    // dereference routine: OK
    void DeRef(PointerHandle<PointerType> * _pPointerHandle)
    {
      if (_pPointerHandle != NULL )
      {
	if (_pPointerHandle->DecRefCnt() == 0)
	{
	  delete _pPointerHandle;
	  _pPointerHandle = NULL;
	}
      }
    }
    
    void MakeObjModifyable()
    {
      if( original == YES )
      {
	
	PointerType pObj_old      =   pPointerHandle->pObj;
	Choice destAtEnd_old      =   pPointerHandle->destAtEnd;
	
	pPointerHandle->pObj      =   CopyObj(pObj_old);
	pPointerHandle->destAtEnd = YES;
	DeRef(pPointerHandle);
	
	pPointerHandle = new PointerHandle<PointerType>(pObj_old,destAtEnd_old);
	
      } else
      {
	
	PointerHandle<PointerType> * pPointerHandle_old = pPointerHandle;
	
	pPointerHandle = new PointerHandle<PointerType>((*CopyObj)(pPointerHandle->pObj),YES);
	
	DeRef(pPointerHandle_old);
	
      }
      
      original = YES;
      
    }
    
  private:
    
    PointerHandle<PointerType> * pPointerHandle;
    Choice original;
    
};

#endif