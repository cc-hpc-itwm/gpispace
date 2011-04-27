#ifndef MEMSTATEHANDLER
#define MEMSTATEHANDLER

  /// standard includes
  #include <stdlib.h>
  #include <iostream>
  #include <ostream>
  #include <assert.h>
  #include <string.h>

/// class Referenced Mem represents a linear block of memory
/// of size MaxSize. It has the ability to count the references
/// made to this block.

/// pData   = pointer to the memory block
/// DatSize = actual size of the data stored in the memory block.
/// MaxSize = total size of the allocated memory
/// refCnt  = counter of the references to the memory block
class ReferencedMem {

public:

	// default constructor
	ReferencedMem() : pData(NULL)
	,DatSize(0)
	,MaxSize(0)
	,allocDat(true)
	,refCnt(0)
	{}

	void Init()
	{
		pData = NULL;
		DatSize = 0;
		MaxSize = 0;
		allocDat = true;
		refCnt = 0;
	}

	// construct a ReferencedMem object with
	// memory allocation of size _DataSize
	ReferencedMem(const size_t & _DatSize) : pData(NULL)
	,DatSize(_DatSize)
	,MaxSize(_DatSize)
	,allocDat(true)
	,refCnt(1)
	{
		pData = allocate(DatSize);
	}

	void Init(const size_t & _DatSize)
	{
		pData = allocate(DatSize);
		DatSize = _DatSize;
		MaxSize = _DatSize;
		if (pData != NULL);
		{
			allocDat = true;
		}
		refCnt = 1;
	}

	// construct a ReferencedMem object with
	// the memory pointed to by _pData of size _DataSize
	ReferencedMem(char * _pData, const size_t & _DatSize) : pData(_pData)
	,DatSize(_DatSize)
	,MaxSize(_DatSize)
	,allocDat(false)
	,refCnt(1)
	{}

	void Init(char * _pData, const size_t & _DatSize)
	{
		pData = _pData;
		DatSize = _DatSize;
		MaxSize = _DatSize;
		allocDat = false;
		refCnt = 1;
	}

	~ReferencedMem()
	{
		if( refCnt == 0 )
		{
			if ( allocDat )
			{
				if ( pData != NULL )
				{
					delete[] pData;
				}
			}
		}
	}

	ReferencedMem(const ReferencedMem & rhs)
	: pData(NULL)
	 ,DatSize(rhs.DatSize)
	 ,MaxSize(rhs.DatSize)
	 ,allocDat(true)
	 ,refCnt(1)
	{
	  pData = allocate(DatSize);
	  memcpy((void *)pData,(void *)rhs.pData,DatSize);
	}

	ReferencedMem & operator=(const ReferencedMem & rhs)
	{
		DatSize=rhs.DatSize;
		pData=allocate(DatSize);
		memcpy((void *)pData,(void *)rhs.pData,DatSize);
		MaxSize=rhs.DatSize;
		allocDat=true;
		refCnt=rhs.refCnt;

	    return *this;
	}

//	ReferencedMem(ReferencedMem & rhs)
//	: pData(rhs.pData)
//     ,DatSize(rhs.DatSize)
//	 ,MaxSize(rhs.MaxSize)
//	 ,allocDat(rhs.allocDat)
//	 ,refCnt(rhs.refCnt + 1)
//	{}

	char * allocate(const size_t & _size)
    									{

		char * pMem;

#ifdef SHOWMEMALLOC
		std::cout<<std::endl;
		std::cout<<std::endl;
		std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
		std::cout<<"!!! MemHandler: Allocating memory  !!!"<<std::endl;
		std::cout<<"!!! Memsize = "<<_size<<" Byte(s)  !!!"<<std::endl;
		std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl<<std::endl;
#endif

		pMem =  new char[_size];

		if(pMem == NULL)
		{
			throw std::runtime_error("ReferencedMem::allocate failed");
		}

		return pMem;

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
			//throw std::runtime_error("ref cnt < 0");
		}
		return (refCnt);
	}


public:

	char * pData; 				        // Pointer to the raw data
	size_t DatSize;						// size of the data
	size_t MaxSize; 				    // allocated size
	bool allocDat;				        // Boolean whether the data
	                                    // was allocated or not
	long refCnt;				        // Reference counting variable

}; // ReferencedMem definition


/// Memory Handler
class MemHandler {

public:

	MemHandler()
	: pReferencedMem(NULL)
	 ,orig(true)
	{ }

	char * Init()
	{
		orig = true;
		DeRef(pReferencedMem);
		pReferencedMem = NULL;
		return getpData();
	}

	MemHandler(const size_t & _DatSize)
	: pReferencedMem(_DatSize == 0 ? NULL : new ReferencedMem(_DatSize))
	 ,orig(true)
	{ }

	char * Init(const size_t & _DatSize)
	{
		orig = true;
		DeRef(pReferencedMem);
		pReferencedMem = _DatSize == 0 ? NULL : new ReferencedMem(_DatSize);
		return getpData();
	}

	MemHandler(char * _pData, const size_t & _DatSize)
	: pReferencedMem(_DatSize == 0 ? NULL : new ReferencedMem(_pData,_DatSize))
	 ,orig(true)
	{ }

	char * Init(char * _pData, const size_t & _DatSize)
	{
		orig = true;
	    DeRef(pReferencedMem);
	    pReferencedMem = _DatSize == 0 ? NULL : new ReferencedMem(_pData,_DatSize);
	    return getpData();
	}

	~MemHandler()
	{
		if ( pReferencedMem != NULL)
		{
			DeRef(pReferencedMem);
		}
	}

	MemHandler(const MemHandler & rhs)
	: pReferencedMem(rhs.getDatSize() == 0 ? NULL : new ReferencedMem(rhs.getDatSize()))
	 ,orig(true)
	{
	  if(pReferencedMem != NULL)
	  {
		  memcpy( (void *)(getpData())
				  ,(void *)(rhs.pReferencedMem->pData)
				  ,rhs.getDatSize() );
	  }
	}

	MemHandler(MemHandler & rhs)
		: pReferencedMem(rhs.pReferencedMem)
		 ,orig(false)
	{
	  pReferencedMem->IncRefCnt();
	}

	MemHandler & operator=(const MemHandler & rhs)
	{
		// rhs points on a constant object
		// we need to make a copy

//		// TEST
//		if (pReferencedMem != rhs.pReferencedMem )
//		{
//
//			DeRef(pReferencedMem);
//
//			pReferencedMem=rhs.pReferencedMem;
//
//			pReferencedMem->IncRefCnt();
//
//			// copy the data
//			memcpy((void *)getpData(),(void *)rhs.pReferencedMem->pData,rhs.getDatSize());
//			//
//			orig = true;
//
//		}

		// END TEST

        if(pReferencedMem == NULL)
        {
        	Init(rhs.pReferencedMem->DatSize);
        }
        else
        {
//        	if(pReferencedMem->allocDat == true)
//        	{
//        		Init(rhs.pReferencedMem->DatSize);
//        	}
//        	else
//        	{
//
//        		if (pReferencedMem->refCnt > 1)
//        		{


        			if( orig == true )
        			{
        				char * pData_old   = pReferencedMem->pData;
        				size_t MaxSize_old = pReferencedMem->MaxSize;
        				*pReferencedMem = *pReferencedMem;
        				Init(pData_old,MaxSize_old);

        			} else
        			{
                        ReferencedMem * pReferencedMem_old = pReferencedMem;
        				pReferencedMem = new ReferencedMem(*pReferencedMem_old);
        				DeRef(pReferencedMem_old);
        			}



//        		}

        		// we are the only one carrying the data now
        		// check that the memory is sufficient for the data
        		if(pReferencedMem->MaxSize < rhs.pReferencedMem->DatSize)
        		{
        			std::cout<<" pMemHandler->MaxSize = "<<pReferencedMem->MaxSize
		                     <<" rhs.pMemHandler->Datsize = "<<rhs.pReferencedMem->DatSize<<std::endl;
        			std::cout<<" pMemHandler->refCnt ="<<pReferencedMem->refCnt<<std::endl;
        			throw std::runtime_error("Memory is not sufficient");
        		}
//
//        	}
        }
		// copy the data
		memcpy((void *)pReferencedMem->pData,(void *)rhs.pReferencedMem->pData,rhs.pReferencedMem->DatSize);

		pReferencedMem->DatSize = rhs.pReferencedMem->DatSize;

		orig=true;

		return *this;
	}

	MemHandler & operator=(MemHandler & rhs)
	{

		// rhs points on a non constant object
		// we do not need to make a copy

		if (pReferencedMem != rhs.pReferencedMem )
		{

			DeRef(pReferencedMem);

			pReferencedMem=rhs.pReferencedMem;

			pReferencedMem->IncRefCnt();

			orig = false;

		}

		return *this;

	}

	void SetpData(char * _pData, const size_t _DatSize)
	{

		if(_pData != pReferencedMem->pData || _DatSize != pReferencedMem->DatSize)
		{

			if(pReferencedMem->refCnt > 1)
			{
				Init(_pData,_DatSize);
			}

			if(pReferencedMem->refCnt == 1)
			{
				pReferencedMem->pData   = _pData;
				pReferencedMem->DatSize = _DatSize;
				orig  = true;
			} else
			{
				std::cout<<"pMemHandler->refCnt ="<<pReferencedMem->refCnt<<std::endl;
			}

		}

	}

	void SetSelfAlloc()
	{
		if( pReferencedMem != NULL)
		{
		  pReferencedMem->allocDat = true;
		}
		else
		{
		  throw std::runtime_error("MemStateHandler::Setting self alloc flag to an uninitialized pMemHandler");
		}
	}

	char * getpData() const
    {
		if (pReferencedMem != NULL)
		{
		  return pReferencedMem->pData;
		}
		else
		{
		  return NULL;
		}

    }

	size_t getDatSize() const
	{
		if (pReferencedMem != NULL)
	    {
		  return pReferencedMem->DatSize;
	    }
		else
		{
		  return 0;
		}
	}


private:

	void DeRef(ReferencedMem * _pMemHandler)
	{
		if (_pMemHandler != NULL )
		{
			if (_pMemHandler->DecRefCnt() == 0)
			{
				delete _pMemHandler;
				_pMemHandler = NULL;
			}
		}
	}



private:

	ReferencedMem * pReferencedMem;
	bool orig;

}; // MemHandler definition

#endif
