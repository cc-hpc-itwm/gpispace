/***************************************************************************
                          receivergrid.h  -  description
                             -------------------
    begin                : Fri Feb 17 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef RECEIVERGRID_H
#define RECEIVERGRID_H


/**
  *@author Dirk Merten
  */
#include "include/types.h"
#include "structures/triangle.h"
#include "structures/grid3d.h"

class ReceiverGrid {
public: 
  /// Iterator over 3-dim integer indicees to access reveiver points.
  class iterator{
    public:
    iterator():RM(NULL){
      i_b = 0; j_b = 0; k_b = 0;
      i_e = 0; j_e = 0; k_e = 0;
      i = 0; j = 0; k = 0;
    };

    /// Initialize iterator from starting indicees i_b, j_b, k_b to end indicees (excluding!) i_e, j_e, k_e
    iterator(const RcvMem* _RM, const int& _i_b, const int& _j_b, const int& _k_b,
                          const int& _i_e, const int& _j_e, const int& _k_e):RM(_RM)
    {
      i_b = _i_b; j_b = _j_b; k_b = _k_b;
      i_e = _i_e; j_e = _j_e; k_e = _k_e;

      if (i_b >= i_e)
        i_b = i_e+1;
        
      if (j_b>=j_e)
      {
        j_b = j_e+1;
        i_b = i_e+1;
      }

      if (k_b>=k_e)
      {
        k_b = k_e+1;
        j_b = j_e+1;
        i_b = i_e+1;
      }
      
      i = i_b; j = j_b; k = k_b;
    };

    void operator++(){
     k++;
     if ( k >= k_e)
     {
       k = k_b;
       j++;
       if ( j >= j_e)
       {
         j = j_b;
         i++;
       }
     }
    };

    Receiver* operator*(){
      return (RM->GetPointAt(i, j, k));
    };

    bool operator!=(const iterator& other){
      return ( (i == other.i) || (j == other.j) || (k == other.k)
               || (i_b == other.i_b) || (j_b == other.j_b) || (k_b == other.k_b)
               || (i_e == other.i_e) || (j_e == other.j_e) || (k_e == other.k_e)
               || (RM != other.RM) );
    };

    bool end(){
      return (i >= i_e);
    };
    //private:
    const RcvMem * RM;

    int i_b, j_b, k_b;
    int i_e, j_e, k_e;
    int i, j, k;
  };  // end of iterator-class

  ReceiverGrid();
  ReceiverGrid(const grid3D& _G);
  ReceiverGrid(const point3D<float>& X0Rcv, const point3D<int>& NRcv, const point3D<float>& dxRcv);
  ~ReceiverGrid();

  Receiver* GetPointAt(const int& x, const int& y, const int& z) const;
  iterator begin() const;
  iterator begin(const Triangle* Tri) const;
  iterator begin(const point3D<float>& X0, const point3D<float>& X1) const;
  void clear();

  /** Getter functions for Nx, Ny, Nz */
  const int& getNx() const {return G.getNx();};
  const int& getNy() const {return G.getNy();};
  const int& getNz() const {return G.getNz();};

  /** Getter functions for X0, X1, dx */
  const point3D<float> getX0() const {return G.GetCoord(0, 0, 0);};
  const point3D<float> getX1() const {return G.GetCoord(G.getNx()-1, G.getNy()-1, G.getNz()-1);};
  const point3D<float> getdx() const {return G.GetCoord(1, 1, 1) - G.GetCoord(0, 0, 0);};

  /** return amount of memory needed */
  unsigned long GetMemoryNeeded() { return sizeof(G)+RcvGrid.GetMemoryNeeded(G.getNx(), G.getNy(), G.getNz()); }


private:
  grid3D G;

  RcvMem RcvGrid;
};

#endif
