/***************************************************************************
                          nodemem.h  -  description
                             -------------------
    begin                : Thu Nov 24 2005
    copyright            : (C) 2005 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef NODEMEM_H
#define NODEMEM_H


/**
  *@author Dirk Merten
  */
#include "Container.h"

#define CONTAINER_SIZE 64
#define CONTAINER_SIZE_LN_2 6
#define CONTAINER_SIZE_MOD 63
template<class d_point>
class NodeMem {
public:

  class iterator{
    public:
    iterator():NM(NULL),Pointer(NULL),Index(0){};
    iterator(NodeMem<d_point>* _NM):NM(_NM),Index(0)
    {
      Pointer = NM->lbu[Index].begin();
    };
    iterator(NodeMem<d_point>* _NM, const int& _I, d_point* _P):NM(_NM),Pointer(_P),Index(_I){};
    iterator(const iterator& other):NM(other.NM),Pointer(other.Pointer),Index(other.Index){};

    void operator++(){
     Pointer++;
     if (Pointer ==  NM->lbu[Index].end())
     {
       Index++;
       if (Index <= NM->_endIndex)
         Pointer = &(NM->lbu[Index][0]);
       else
         Pointer = NULL;
     }
    };
    d_point& operator*(){
      return (*Pointer);
    };
    bool operator!=(const iterator& other){
      return ( (Index != other.Index) || (Pointer != other.Pointer) || (NM != other.NM) );
    }
    //private:
    NodeMem<d_point> * NM;
    d_point* Pointer;
    int Index;
  };
  
  NodeMem();
  NodeMem(const int& _dim2);
  ~NodeMem();
  /** operator [] (const MemMan3D_bucket& ) */
  d_point & operator[](const int & i);
  /** No descriptions */
  iterator begin(){
    return iterator(this, 0, lbu[0].begin());
  };
  /** No descriptions */
  iterator end(){
    return iterator(this, _endIndex, &lbu[_endIndex][_endSubIndex]);
/*     return iterator(this, _endIndex + 1, NULL); */
  };
  /** No descriptions */
  int size();
  /** No descriptions */
  int reserve(const int& N);
  /** No descriptions */
  d_point* push_back(d_point dp);
  /** No descriptions */
  void clear();
  /** No descriptions */
  int GetIndex(const int& i, int& SubIndex);
  /** No descriptions */                                                      
  int GetIndex(const int& i);
  /** No descriptions */
  int GetSubIndex(const int& i);
  /** No descriptions */
  void InitRun();
  /** No descriptions */
  d_point* NextIndex();
private: // Private attributes
  /** */
  container<d_point>* lbu;
  /**  */
  int _size;
  int _capacity;
  /** Pointer to address behind last element */
  d_point* _end;
  int _endIndex, _endSubIndex;
  /** */
  int N1, N2;
  /** Width of a 2-dim Block*/
  int dim2;

  int runIndex, runSubIndex;

private:
  d_point* next(int& Index, int& SubIndex);

  friend class iterator;
};


template<class d_point>
NodeMem<d_point>::NodeMem(){
  lbu = NULL;
  _size = 0;
  _capacity = 0;
  _end = NULL;
  _endIndex = 0; _endSubIndex = 0;
  dim2 = CONTAINER_SIZE;
  //  std::cout << "sizeof d_point is " << sizeof(d_point) << std::endl;
}
template<class d_point>
NodeMem<d_point>::NodeMem(const int& _dim2){
#ifdef DEBUG
    if (_dim2 <= 0)
	std::cout << "_dim2 <= 0\n";
#endif
  lbu = NULL;
  _size = 0;
  _capacity = 0;
  _end = NULL;
  _endIndex = 0; _endSubIndex = 0;
  dim2 = _dim2;
  //  std::cout << "sizeof d_point is " << sizeof(d_point) << std::endl;
}

template<class d_point>
NodeMem<d_point>::~NodeMem(){
  if (lbu != NULL)
    delete[] lbu;
}

/** No descriptions */
template<class d_point>
void NodeMem<d_point>::clear(){
  if (lbu != NULL)  delete[] lbu;

  lbu = NULL;
  _size = 0;
  _capacity = 0;
  _end = NULL;
  _endIndex = 0; _endSubIndex = 0;
}

/** No descriptions */
template<class d_point>
int NodeMem<d_point>::size(){
  return _size;
}
/** The memory for N elements is allocated and the bookkeeping is initialized. */
template<class d_point>
int NodeMem<d_point>::reserve(const int& N){

  N1 = ((N+dim2-1)/dim2 + CONTAINER_SIZE-1) / CONTAINER_SIZE;
  N2 = (dim2 + CONTAINER_SIZE-1) / CONTAINER_SIZE;

  int ret = 0;
  lbu = new container<d_point>[N1 * N2];
  if (lbu == NULL)
  {
    std::cerr << "ERROR in NodeMem<>::reserve(): Could not allocate Memory 1\n";
    ret = 1;
  }
  
  for (int i = 0; i < N1*N2; i++)
    {
      lbu[i].resize( CONTAINER_SIZE * CONTAINER_SIZE);
      if ( lbu[i].begin() == NULL)
        {
          std::cerr << "ERROR in NodeMem<>::reserve(): Could not allocate Memory 2\n";
          ret = 1;
        }
    }

  _capacity = N; //N1*N2*CONTAINER_SIZE*CONTAINER_SIZE;
  _end = &lbu[0][0];
  
  //  std::cout << "Allocated memory: " << N1*N2 << " * "
  //                                    << CONTAINER_SIZE * CONTAINER_SIZE << " * "
  //                                    << sizeof(d_point) << std::endl;

  return ret;
}
template<class d_point>
d_point* NodeMem<d_point>::push_back(d_point dp){
  if (_size < _capacity)
  {
//#ifdef DEBUG_MEM
//    int SubIndex;
//    const int Index = GetIndex(_size, SubIndex);
//    cout << _size << " : " << Index << " =? " << _endIndex << ", " << SubIndex << " =? " << _endSubIndex << endl;
//#endif
//
//    (*_end) = dp;
//    d_point* _ret = _end;
//
//    _size++;
//
//    _end = next(_endIndex, _endSubIndex);
//    return _ret;

    int SubIndex;
    const int Index = GetIndex(_size, SubIndex);
    lbu[Index][SubIndex] = dp;
    _size++;

    _endIndex = GetIndex(_size, _endSubIndex);
    _end = &lbu[Index][SubIndex];
    return &lbu[Index][SubIndex];
  }
  else
  {
    std::cerr << "NodeMem::push_back did not work for size = " << _size << std::endl;
    return NULL;
  }
}
/** operator [] (const MemMan3D_bucket& ) */
template<class d_point>
d_point & NodeMem<d_point>::operator[](const int & i){
  if (i >= _size)
  {
    _size++;
    _end = next(_endIndex, _endSubIndex);
  }

  int SubIndex;
  const int Index = GetIndex(i, SubIndex);
  return lbu[Index][SubIndex];
}

/** No descriptions */
template<class d_point>
int NodeMem<d_point>::GetIndex(const int& i, int& SubIndex){
  const int col = i % dim2;
  const int row = i / dim2;

  const int l_row = row >> CONTAINER_SIZE_LN_2;
  const int l_col = col >> CONTAINER_SIZE_LN_2;

  const int lbu_row = row & CONTAINER_SIZE_MOD;
  const int lbu_col = col & CONTAINER_SIZE_MOD;

  SubIndex = (lbu_row << CONTAINER_SIZE_LN_2) + lbu_col;
  const int Index = l_row * N2 + l_col;

#ifdef DEBUG_MEM
  std::cout << i << ": (" << col << "," << row << ") ("
                     << l_col << "," << l_row << ") ("
                     << lbu_col << "," << lbu_row << ") -> ("
                     << Index << "," << SubIndex << ")" << std::endl;
#endif

  return Index;
}
/** No descriptions */
template<class d_point>
int NodeMem<d_point>::GetIndex(const int& i){
  const int col = i % dim2;
  const int row = i / dim2;

  const int l_row = row >> CONTAINER_SIZE_LN_2;
  const int l_col = col >> CONTAINER_SIZE_LN_2;

  const int Index = l_row * N2 + l_col;
  return Index;
}
/** No descriptions */
template<class d_point>
int NodeMem<d_point>::GetSubIndex(const int& i){
  int col = i % dim2;
  int row = i / dim2;

  const int lbu_row = row & CONTAINER_SIZE_MOD;
  const int lbu_col = col & CONTAINER_SIZE_MOD;

  int SubIndex = (lbu_row << CONTAINER_SIZE_LN_2) + lbu_col;
  return SubIndex;
}

/* Sets Index und SubIndex to the next element and returns a pointer to that. */
template<class d_point>
d_point* NodeMem<d_point>::next(int& Index, int& SubIndex){

  SubIndex++;
  if ( (((Index+1) % N2) == 0) && ( (SubIndex & CONTAINER_SIZE_MOD) == (dim2 & CONTAINER_SIZE_MOD)) )
    SubIndex = (( ((SubIndex-1) >> CONTAINER_SIZE_LN_2) + 1 ) << CONTAINER_SIZE_LN_2);

  if ( (SubIndex >= CONTAINER_SIZE * CONTAINER_SIZE)
    || (( (Index / N2) == (N1-1)) && ( SubIndex >= ( ((_capacity+dim2-1) / dim2) << CONTAINER_SIZE_LN_2))))
  {
    Index++;
    SubIndex = 0;
  }

  //cout << "runIndex = " << runIndex << ", runSubIndex = " << runSubIndex << endl;
  return &lbu[Index][SubIndex];
}

template<class d_point>
void NodeMem<d_point>::InitRun(){
  runIndex = 0;
  runSubIndex = 0;
}

template<class d_point>
d_point* NodeMem<d_point>::NextIndex(){

  d_point* d_ptr = &lbu[runIndex][runSubIndex];

  runSubIndex++;
  if ( (((runIndex+1) % N2) == 0) && ( (runSubIndex & CONTAINER_SIZE_MOD) == (dim2 & CONTAINER_SIZE_MOD)))
    runSubIndex = (( (runSubIndex >> CONTAINER_SIZE_LN_2) + 1 ) << CONTAINER_SIZE_LN_2);

  if (runSubIndex >= lbu[runIndex].size())
  {
    runIndex++;
    runSubIndex = 0;
  }

  //cout << "runIndex = " << runIndex << ", runSubIndex = " << runSubIndex << endl;
  return d_ptr;
}

/** No descriptions */
//template<class d_point>
//NodeMem<d_point>::iterator NodeMem<d_point>::begin(){
//  return iterator(this, 0, 0);
//}

/** No descriptions */
//template<class d_point>
//NodeMem<d_point>::iterator NodeMem<d_point>::end(){
//  return iterator(this, _endIndex, _endSubIndex);
//}
#endif
