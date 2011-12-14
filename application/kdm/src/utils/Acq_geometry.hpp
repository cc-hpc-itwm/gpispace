#ifndef ACGEOM
#define ACGEOM

#include <iostream>
#include <fstream>
#include <sstream>
#include "structures/Point2d.hpp"
#include "XMLReader_red.h"
#include "structures/geomdefstruct.hpp"
#include "structures/deg.h"
#include "include/defs.h"
//#include LOGGERINCLUDE

/* ################################################################## */
/* ################################################################## */
template<class T> class Acq_geometry {
  /* includes:
     geometry specified via the struct geom_def_struct,
     mapping from/to world, model, inline/xline coordinates.

     The link between world and inline/xline coordinates is given by one point
     for which inline/xline number and world coordinates are specified:
     geom_def_struct::one_inline_num, one_xline_num for which
     geom_def_struct::x_one, y_one

     P_orig is the world coordinates of
     geom_def_struct::first_inline_num, first_xline_num

     model coordinates:
     origin (0, 0) is at P_orig,
     xline indices count positive in positive x-direction,
     inline indices count positive in positive y-direction.
  */


// public functions
public:
    Acq_geometry();
    Acq_geometry(const geomdefstruct&);
    ~Acq_geometry();

    void inlxl_to_WORLDxy(int, int, T *, T *);
    void MODxy_to_WORLDxy(T, T, T *, T *);
    void WORLDxy_to_MODxy(T, T, T *, T *);
    void WORLDxy_to_inlxl(T, T, int *, int *);

    void WORLDxy_to_floatinlxl(Point2d<T>, T *, T *);
    void WORLDxy_to_floatinlxl(T, T, T *, T *);

    Point2d<T> inlxl_to_WORLDxy(int, int);
    Point2d<T> MODxy_to_WORLDxy(Point2d<T>);

    const geomdefstruct& getGeom(){return geom;}

// private funcitons
private:
    Acq_geometry(ifstream &);
    //Acq_geometry(XMLReader &);

    int read(ifstream &);
//  int read(XMLReader &);
    int read(const geomdefstruct &);
    int show(ofstream &);

  int init();


  T get_mig_extension_ininline(int, int);
  T get_mig_extension_inxline(int, int);

  bool is_valid_inline(int inline_external);
  bool is_valid_xline(int xline_external);

  Point2d<T> rotate_to_WORLDxy(Point2d<T> P);
  Point2d<T> rotate_to_MODxy(Point2d<T> P);
  void rotate_to_WORLDxy(T x_in, T y_in, T * x_out, T * y_out);
  void rotate_to_MODxy(T x_in, T y_in, T * x_out, T * y_out);

  void inlxl_to_WORLDxy(int, T, int, T, T *, T *);

  Point2d<T> inlxl_to_WORLDxy(int, T, int, T);
  Point2d<T> WORLDxy_to_MODxy(Point2d<T>);
  void WORLDxy_to_inlxl(Point2d<T>, int *, int *);

  int return_xline(T x_MODxy);
  int return_xline10(T x_MODxy);
  int return_inline(T y_MODxy);
  int return_inline10(T y_MODxy);


// public variables
public:

// private variables
private:

  bool complete;

  /* input */
  geomdefstruct geom;
  /* */

  /* computed */
  int last_inline_num, last_xline_num;
  T extension_in_inline_dir, extension_in_xline_dir;
  T angle_rot_rad;
  T dirflip;
  Point2d<T> P_one; /* location of intersection of one_inline_num and one_xline_num */
  Point2d<T> P_orig; /* location of intersection of first_inline_num and first_xline_num */
  Point2d<T> pdir1, pdir2, npdir1, npdir2;
  T cosa, sina;
  T A[2][2];
  T A_inv[2][2];
  /* */

  //  LOGGER;
};



template <class T>
Acq_geometry<T>::Acq_geometry() {
  complete = false;
  /* needs to call
     read(  );
     init();
     explicitely in order to use Acq_geometry */
}
template <class T>
Acq_geometry<T>::Acq_geometry(ifstream & inp) {
  read(inp);
  init();
}
// template <class T>
// Acq_geometry<T>::Acq_geometry(XMLReader & xr) {
//   read(xr);
//   init();
// }
template <class T>
Acq_geometry<T>::Acq_geometry(const geomdefstruct& g) {
  geom = g;
  init();
}
template <class T>
Acq_geometry<T>::~Acq_geometry() {
  ;
}



template <class T>
int Acq_geometry<T>::read(ifstream & inp) {
  string saux;

  inp >> saux >> geom.n_inlines;
  inp >> saux >> geom.n_inlines;
  inp >> saux >> geom.first_inline_num;
  inp >> saux >> geom.first_xline_num;
  inp >> saux >> geom.one_inline_num;
  inp >> saux >> geom.one_xline_num;
  inp >> saux >> geom.x_one;
  inp >> saux >> geom.y_one;
  inp >> saux >> geom.d_between_inlines;
  inp >> saux >> geom.d_between_xlines;
//  inp >> saux >> geom.angle_rot_deg;
  inp >> saux >> geom.inlinedir;

  return 0;
}
// template <class T>
// int Acq_geometry<T>::read(XMLReader & xr) {
//   int return_value = 0;
//   string path1 = "seismic/";
//   string path2 = "seismic/Geometry/";

//   if ( !xr.getInt(path2+"n_inlines",geom.n_inlines) ) {
//       USERERRORLOG("Input Error: %s n_inlines not specified",path2);
//       return_value = 1;
//   }
//   if ( !xr.getInt(path2+"n_xlines",geom.n_xlines) ) {
//    USERERRORLOG("Input Error: %s\n_xlines not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getInt(path2+"first_inline_num",geom.first_inline_num) ) {
//    USERERRORLOG("Input Error: %sfirst_inline_num not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getInt(path2+"first_xline_num",geom.first_xline_num) ) {
//    USERERRORLOG("Input Error: %sfirst_xline_num not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getInt(path2+"one_inline_num",geom.one_inline_num) ) {
//    USERERRORLOG("Input Error: %sone_inline_num not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getInt(path2+"one_xline_num",geom.one_xline_num) ) {
//    USERERRORLOG("Input Error: %sone_xline_num not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getFloat(path2+"one_x",geom.x_one) ) {
//    USERERRORLOG("Input Error: %sone_x not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getFloat(path2+"one_y",geom.y_one)) {
//    USERERRORLOG("Input Error: %sone_y not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getFloat(path2+"d_between_inlines",geom.d_between_inlines) ) {
//    USERERRORLOG("Input Error:  %sd_between_inlines not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getFloat(path2+"d_between_xlines",geom.d_between_xlines) ) {
//    USERERRORLOG("Input Error: %sd_between_xlines not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getFloat(path2+"angle_rot_deg",geom.angle_rot_deg) ) {
//    USERERRORLOG("Input Error: %sangle_rot_deg not specified",&path2);
//     return_value = 1;
//   }
//   if ( !xr.getInt(path2+"xlinedir",geom.xlinedir)) {
//    USERERRORLOG("Input Error: %sxlinedir not specified",&path2);
//     return_value = 1;
//   }

//   return return_value;
// }

template <class T>
int Acq_geometry<T>::show(ofstream & infofile) {
  ostringstream isa;

  isa << "     acquisition parameters:" << endl <<
    "      n_inlines         = " << geom.n_inlines <<  endl <<
    "      n_xlines          = " << geom.n_xlines <<  endl <<
    "      first_inline_num  = " << geom.first_inline_num <<  endl <<
    "      first_xline_num   = " << geom.first_xline_num <<  endl <<
    "      one_inline_num   = " << geom.one_inline_num <<  endl <<
    "      one_xline_num    = " << geom.one_xline_num <<  endl <<
    "      x_one          = " << geom.x_one <<  endl <<
    "      y_one          = " << geom.y_one <<  endl <<
    "      d_between_inlines = " << geom.d_between_inlines <<  endl <<
    "      d_between_xlines  = " << geom.d_between_xlines <<  endl <<
//    "      angle_rot_deg     = " << geom.angle_rot_deg <<  endl <<
    "      inlinedir              = " << geom.inlinedir << endl <<
       "-------------------------" << endl;
  infofile << isa.str();
  cout << isa.str();

  return 0;
}


template <class T>
int Acq_geometry<T>::init() {

  angle_rot_rad = deg2rad(geom.angle_rot_deg);

  cosa = cos(angle_rot_rad);
  sina = sin(angle_rot_rad);

  /* row index, column index */
  A[0][0] = cosa;
  A[1][0] = -sina;
  A[0][1] = -A[1][0];
  A[1][1] = A[0][0];

  A_inv[0][0] = A[0][0];
  A_inv[1][1] = A[1][1];
  A_inv[1][0] = -A[1][0];
  A_inv[0][1] = -A[0][1];

  last_inline_num = geom.first_inline_num + geom.n_inlines-1;
  last_xline_num = geom.first_xline_num + geom.n_xlines-1;

  extension_in_inline_dir =  (geom.n_inlines-1) * geom.d_between_xlines;
  extension_in_xline_dir =  (geom.n_xlines-1) * geom.d_between_inlines;

  npdir1.x[0] = cos(angle_rot_rad);
  npdir1.x[1] = sin(angle_rot_rad);
  npdir2 = npdir1.rotr90();

  pdir1 = npdir1.scalep2(extension_in_inline_dir );
  pdir2 = npdir2.scalep2(extension_in_xline_dir );

  dirflip = geom.inlinedir;

  P_one.setvalue(geom.x_one, geom.y_one);
  P_orig = inlxl_to_WORLDxy(geom.first_inline_num, geom.first_xline_num);

  complete = true;

  return 0;
}

template <class T>
bool Acq_geometry<T>::is_valid_inline(int inline_external) {
  if ( inline_external < geom.first_inline_num )
    return false;
  if ( inline_external > last_inline_num )
    return false;
  return true;
}
template <class T>
bool Acq_geometry<T>::is_valid_xline(int xline_external) {
  if ( xline_external < geom.first_xline_num )
    return false;
  if ( xline_external > last_xline_num )
    return false;
  return true;
}

template <class T>
int Acq_geometry<T>::return_xline(T x_MODxy) {
  /* x_MODxy given in model coordinate system */
  int m = static_cast<int> ( x_MODxy / geom.d_between_xlines)
    + geom.first_xline_num;
  return m;
}
template <class T>
int Acq_geometry<T>::return_xline10(T x_MODxy) {
  /* x_MODxy given in model coordinate system */
  int m = static_cast<int> (10.0 * x_MODxy / geom.d_between_xlines)
    + 10*geom.first_xline_num;
  return m;
}
template <class T>
int Acq_geometry<T>::return_inline(T y_MODxy) {
  /* y_MODxy given in model coordinate system */
  int m = static_cast<int> ( y_MODxy / geom.d_between_inlines)
    + geom.first_inline_num;
  return m;
}
template <class T>
int Acq_geometry<T>::return_inline10(T y_MODxy) {
  /* y_MODxy given in model coordinate system */
  int m = static_cast<int> (10.0 * y_MODxy / geom.d_between_inlines)
    + 10*geom.first_inline_num;
  return m;
}


template <class T>
Point2d<T> Acq_geometry<T>::MODxy_to_WORLDxy(Point2d<T> P) {
  return( rotate_to_WORLDxy(P) + P_orig );
}

template <class T>
Point2d<T> Acq_geometry<T>::WORLDxy_to_MODxy(Point2d<T> P) {
  return( rotate_to_MODxy(P-P_orig));
}
template <class T>
void Acq_geometry<T>::MODxy_to_WORLDxy(T x_in, T y_in, T * x_out, T * y_out) {
  Point2d<T> P;
  P.setvalue(x_in, y_in);
  P = MODxy_to_WORLDxy(P);
  *x_out = P.x[0];
  *y_out = P.x[1];
}

template <class T>
void Acq_geometry<T>::WORLDxy_to_MODxy(T x_in, T y_in, T * x_out, T * y_out) {
  Point2d<T> P;
  P.setvalue(x_in, y_in);
  P = WORLDxy_to_MODxy(P);
  *x_out = P.x[0];
  *y_out = P.x[1];
}

template <class T>
void Acq_geometry<T>::WORLDxy_to_inlxl(Point2d<T> P, int *il, int *xl) {
  Point2d<T> Paux = WORLDxy_to_MODxy(P);
  *xl = (int)(Paux.x[0] / geom.d_between_xlines + geom.first_xline_num + 0.5);
  *il = (int)(Paux.x[1] / geom.d_between_inlines + geom.first_inline_num + 0.5);
}
template <class T>
void Acq_geometry<T>::WORLDxy_to_inlxl(T x_in, T y_in, int *il, int *xl) {
  Point2d<T> P;
  P.setvalue(x_in, y_in);
  WORLDxy_to_inlxl(P, il, xl);
}
template <class T>
void Acq_geometry<T>::WORLDxy_to_floatinlxl(Point2d<T> P, T *il, T *xl) {
  Point2d<T> Paux = WORLDxy_to_MODxy(P);
  *xl = Paux.x[0] / geom.d_between_xlines + (T)geom.first_xline_num;
  *il = Paux.x[1] / geom.d_between_inlines + (T)geom.first_inline_num;
}
template <class T>
void Acq_geometry<T>::WORLDxy_to_floatinlxl(T x_in, T y_in, T *il, T *xl) {
  Point2d<T> P;
  P.setvalue(x_in, y_in);
  WORLDxy_to_floatinlxl(P, il, xl);
}

template <class T>
Point2d<T> Acq_geometry<T>::inlxl_to_WORLDxy(int inline_nr, int xline_nr) {
  /* returns x-, y- world coordinates for inline-crossline number */
  Point2d<T> P;

  P.setvalue(
	     static_cast<T>( xline_nr  -  geom.one_xline_num) *  geom.d_between_xlines,
	     static_cast<T>( inline_nr - geom.one_inline_num) * geom.d_between_inlines
	     );

  P = rotate_to_WORLDxy(P);
  P = P + P_one; /* the alternative to use P_orig here and geom.first_?line_num above
		    is not used because this routine is used for computing P_orig ! */

  return P;
}
template <class T>
Point2d<T> Acq_geometry<T>::inlxl_to_WORLDxy(int inline_nr_a, T ylen, int xline_nr_a, T xlen) {
  /* returns x-, y- world coordinates for node located xl away from inline_nr_a
     and yl away from xline_nr_a */

  Point2d<T> P, P1;

  P = inlxl_to_WORLDxy(inline_nr_a, xline_nr_a);

  P1.setvalue(xlen, ylen);

  P1 = rotate_to_WORLDxy(P1);

  P = P + P1;

  return P;
}
template <class T>
void Acq_geometry<T>::inlxl_to_WORLDxy(int inline_nr, int xline_nr,
				       T * x_out, T * y_out) {
  /* returns x-, y- world coordinates for inline-crossline number */
  Point2d<T> P = inlxl_to_WORLDxy(inline_nr, xline_nr);
  *x_out = P.x[0];
  *y_out = P.x[1];
}
template <class T>
void Acq_geometry<T>::inlxl_to_WORLDxy(int inline_nr_a, T ylen, int xline_nr_a, T xlen,
				       T * x_out, T * y_out) {
  /* returns x-, y- world coordinates for node located xl away from inline_nr_a
     and yl away from xline_nr_a */
  Point2d<T> P = inlxl_to_WORLDxy(inline_nr_a, ylen, xline_nr_a, xlen);
  *x_out = P.x[0];
  *y_out = P.x[1];
}




template <class T>
T Acq_geometry<T>::get_mig_extension_ininline(int mig1_xline , int mig2_xline) {
  return ( static_cast<T>(mig2_xline - mig1_xline ) * geom.d_between_xlines);
}
template <class T>
T Acq_geometry<T>::get_mig_extension_inxline(int mig1_inline , int mig2_inline) {
  return ( static_cast<T>(mig2_inline - mig1_inline ) * geom.d_between_inlines);
}

template <class T>
Point2d<T> Acq_geometry<T>::rotate_to_WORLDxy(Point2d<T> P) {
  /* rotates into x-, y- world coordinate system */
  Point2d<T> P_out;
  T Px1 = dirflip * P.x[1];

  P_out.x[0] = A_inv[0][0] * P.x[0] + A_inv[0][1] * Px1;
  P_out.x[1] = A_inv[1][0] * P.x[0] + A_inv[1][1] * Px1;

  return P_out;
}
template <class T>
Point2d<T> Acq_geometry<T>::rotate_to_MODxy(Point2d<T> P) {
  /* rotates from x-, y- world into model coordinate system */
  Point2d<T> P_out;

  P_out.x[0] =             A[0][0] * P.x[0] + A[0][1] * P.x[1];
  P_out.x[1] = dirflip * ( A[1][0] * P.x[0] + A[1][1] * P.x[1] );

  return P_out;
}
template <class T>
void Acq_geometry<T>::rotate_to_WORLDxy(T x_in, T y_in, T * x_out, T * y_out) {
  /* rotates into x-, y- world coordinate system */
  Point2d<T> P;
  P.setvalue(x_in, y_in);
  Point2d<T> P_out = rotate_to_WORLDy(P);
  *x_out = P_out.x[0];
  *y_out = P_out.x[1];
}
template <class T>
void Acq_geometry<T>::rotate_to_MODxy(T x_in, T y_in, T * x_out, T * y_out) {
  /* rotates from x-, y- world into model coordinate system */
  Point2d<T> P;
  P.setvalue(x_in, y_in);
  Point2d<T> P_out = rotate_to_MODxy(P);
  *x_out = P_out.x[0];
  *y_out = P_out.x[1];
}


#endif
