/* 
   Copyright by ITWM (2000)

   Purpose:    

   Author(s):  Peter Klein 

   Version:    

   Date:       

   Changes to previous release:

   Maintainer(s) remarks:

*/


#ifndef _CONTAINER_H_
#define _CONTAINER_H_ 

#define CACHELINESIZE 32

#ifdef INSTRUMENT
#include <instrumentation.h>
#endif

#include <cstddef>
#include <iostream>
#include <stdlib.h>


template< class T >
class container
{
#ifdef INSTRUMENT
      instrumentation inst;
#endif 

      int _capac, _size;
      int cachelines;
      T* rep;
      T* _end;

   public: 
      // Default constructor
      container ();

      // Resource aquisitation is initialization
      container ( int n );

      // Deep copy semantics
      container( const container& C );

      // Deep assignement semantics
      container& operator=( const container& C);

      // Destructor
      ~container();

      void resize( int new_capac );

      T* begin(){ return rep; };

      T* end() { return _end; };

      T& operator [] ( int j )
      {
        if (j>=_size)
        {
          _size = (j+1);
          _end = rep + _size;
        }
        return *(rep + j);
//   return rep[j];
    };

      int size(){ return _size;};
      int capacity(){ return _capac;};

      // F must provide void F::operator()(T&) 
      template <class F> 
        F& DoIt( F& g );

};

template< class T >
container< T >::container ():_size(0),_capac(0),rep(0),_end(0){}

template< class T >
container< T >::container( int n):_size(0),_capac(n),cachelines( sizeof(T)*_capac/CACHELINESIZE  )
{
  if( n > 0 )
      rep = (T *) (malloc( sizeof(T) * n ));
  else
      rep = 0;

  _end = rep;
} 



template< class T >
container< T >::container( const container& C ):_size(C._size), _capac(C._capac),cachelines(C.cachelines)
{
  if( _capac !=  0 )
    {
      rep = (T *) (malloc( sizeof(T)*_capac ));

      _end = rep + _size; 
      memcpy( rep, C.rep, sizeof(T)*_size );
    }
  else
    {
      rep = _end = 0;
    }
}



template< class T >
container<T>& container<T>::operator=( const container<T>& C )
{
  if( this != &C ){
    _size = C._size;
    _capac = C._capac;
    cachelines = C.cachelines;

    free( rep ); 

    if( _capac != 0 )
      {
	rep = (T *) malloc( sizeof(T)*_capac );
	_end = rep + _size; 

	memcpy( rep, C.rep,  sizeof(T)*_size );
      }
    else
      {
	rep = _end = 0;
      }
  }
  return *this;
}


template< class T >
container< T >::~container()
{
  if (rep != 0)
    free( rep ); 
}


template< class T >
void container< T >::resize( int new_capac )
{
  if( _capac == 0 )
    {
      _capac = new_capac;
      if( _capac != 0 )
	rep = (T *) malloc( sizeof(T) *  _capac );
      else
	rep = 0;
    }
  else
    {
      T* rep_tmp = (T *) malloc( sizeof(T) *  new_capac );
      int size_tmp = std::min( new_capac, _size );
      memcpy( rep_tmp, rep,  sizeof(T) * size_tmp );

      free( rep ); 
      rep = rep_tmp;
      _capac = new_capac;
      _size = size_tmp;
    }
  _end = rep +  _size;
}





template< class T >
template <class F>  F& 
container< T >::DoIt( F& g )
{
   for ( register T*__restrict__  I( rep ); I!=_end; ++I) g(*I);
   return g;
}


#endif
