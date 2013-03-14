/***************************************************************************
                          coordinates.h  -  description

    With the dawn of UTM/MOD coordinate classes
    it was necessary to have an improved struct for
    bundles of coordinates based on templates of (e.g., UTM or MOD) data

                          -------------------
    begin                : Wed Jul 08 2009
    copyright            : (C) 2009 by Dominik Michel
    email                : micheld@itwm.fhg.de
***************************************************************************/


#ifndef COORDINATES_H
#define COORDINATES_H

/**
 *@author Dominik Michel
 */
template<class T> class Coords_cdp_offset;

template<class T> class Coords_src_recv {

// public methods
public:
  Coords_src_recv() {};
  Coords_src_recv(T a,T b,T c,T d) { SrcX=a; SrcY=b; RcvX=c; RcvY=d; };
  Coords_src_recv(const Coords_src_recv<T>& A) { SrcX=A.SrcX; SrcY=A.SrcY; RcvX=A.RcvX; RcvY=A.RcvY; };
  ~Coords_src_recv() {};

  Coords_src_recv<T>& operator= (const Coords_src_recv<T>& A){
	SrcX=A.SrcX; SrcY=A.SrcY; RcvX=A.RcvX; RcvY=A.RcvY; return *this;
  };

  Coords_cdp_offset<T> Trafo() const {
    Coords_cdp_offset<T> CDP_Offset_xy;
    CDP_Offset_xy.CDPx = 0.5*(SrcX + RcvX);
    CDP_Offset_xy.CDPy = 0.5*(SrcY + RcvY);
    CDP_Offset_xy.Offx = RcvX - SrcX;
    CDP_Offset_xy.Offy = RcvY - SrcY;
    return CDP_Offset_xy;
  };

// public attributes
public:
  T SrcX, SrcY, RcvX, RcvY;

// private methods
 private:

// private attributes
 private:

};

template<class T> class Coords_cdp_offset {

// public methods
public:
  Coords_cdp_offset() {};
  Coords_cdp_offset(T a,T b,T c,T d) { CDPx=a; CDPy=b; Offx=c; Offy=d; };
  Coords_cdp_offset(const Coords_cdp_offset<T>& A) { CDPx=A.CDPx; CDPy=A.CDPy; Offx=A.Offx; Offy=A.Offy; };
  ~Coords_cdp_offset() {};

  Coords_cdp_offset<T>& operator= (const Coords_cdp_offset<T>& A){
	CDPx=A.CDPx; CDPy=A.CDPy; Offx=A.Offx; Offy=A.Offy; return *this;
  };

  Coords_src_recv<T> Trafo() const {
    Coords_src_recv<T> Src_Recv_xy;
    Src_Recv_xy.SrcX = CDPx - 0.5*Offx;
    Src_Recv_xy.RcvX = CDPx + 0.5*Offx;
    Src_Recv_xy.SrcY = CDPy - 0.5*Offy;
    Src_Recv_xy.RcvY = CDPy + 0.5*Offy;
    return Src_Recv_xy;
  };


// public attributes
public:
  T CDPx, CDPy, Offx, Offy;

// private methods
 private:

// private attributes
 private:

};

#endif
