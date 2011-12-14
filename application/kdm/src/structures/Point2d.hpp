#ifndef POINT2d
#define POINT2d

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <iomanip>

using namespace std;

template<class T> class Point2d {
public:
  Point2d(){};
  ~Point2d(){};

  T x[2];


  Point2d<T> operator- (Point2d<T> p1);
  Point2d<T> operator+ (Point2d<T> p1);
  void scalep(T lambda);
  Point2d<T> scalep2(T lambda);
  T len();
  T len2();
  Point2d<T> plinearp(T lambda, Point2d<T> & r);
  void plinearp2(T lambda, Point2d<T> & r);
  void pinvert();
  Point2d<T> pinvert2();
  T crossp(Point2d<T> p2 );
  T crossp2(Point2d<T> p2 );
  T scalarp(Point2d<T> p2 );
  T projectp(Point2d<T> r0);
  void setvalue(T value);
  void setvalue(T value_x, T value_y);
  void rot90();
  Point2d<T> rotr90();
  void minP(Point2d<T>);
  void minP(Point2d<T>, int);
  bool isthesame(Point2d<T>, T);
};

template <class T>
ostream & operator<<(ostream & s, Point2d<T>  & p) {
  s << setw(11) << p.x[0] << " " << setw(11) << p.x[1];
  return s;
}

template <class T>
int readp(ifstream & s, Point2d<T> & p, string sep) {
  char c = ' ';
  s >> p.x[0];
  if ( s.eof() != 0 ) return(-1);
  while (c != *sep.c_str()) {
    s >> c;
    if (c == '\n') return(0);
  }
  s >> p.x[1];
  return(1);
}

template <class T>
int operator>>(ifstream & s, Point2d<T> & p) {
  char c = ' ';
  s >> p.x[0];
  if ( s.eof() != 0 ) return(-1);
  while (c != ';') {    s >> c;    if (c == '\n') return(0);
  }
  s >> p.x[1];
  return(1);
}



template <class T>
Point2d<T> Point2d<T>::operator- (Point2d<T> p1) {
  Point2d<T> p;
  p.x[0] = x[0]-p1.x[0];    p.x[1] = x[1]-p1.x[1];
  return p;
}
template <class T>
Point2d<T> Point2d<T>::operator+ (Point2d<T> p1) {
  Point2d<T> p;
  p.x[0] = x[0]+p1.x[0];    p.x[1] = x[1]+p1.x[1];
  return p;
}
template <class T>
void Point2d<T>::scalep(T lambda) {
  x[0] *= lambda;  
  x[1] *= lambda;
}
template <class T>
Point2d<T> Point2d<T>::scalep2(T lambda) {
  Point2d<T> p;
  p.x[0] = x[0]*lambda;  
  p.x[1] = x[1]*lambda;
  return p;
}
template <class T>
T Point2d<T>::len() {  
  return( sqrt(  pow(x[0], 2.0) +  pow(x[1], 2.0) ) );
}
template <class T>
T Point2d<T>::len2() {  
  return( pow(x[0], 2.0) +  pow(x[1], 2.0) );
}
template <class T>
Point2d<T> Point2d<T>::plinearp(T lambda, Point2d<T> & r) {
  Point2d<T> p;
  p.x[0] = x[0] + lambda * r.x[0];  p.x[1] = x[1] + lambda * r.x[1];
  return p;
}
template <class T>
void Point2d<T>::plinearp2(T lambda, Point2d<T> & r) {
  x[0] += lambda*r.x[0];  x[1] += lambda*r.x[1];
}
template <class T>
void Point2d<T>::pinvert() {
  x[0] = -x[0];
  x[1] = -x[1];
}
template <class T>
Point2d<T> Point2d<T>::pinvert2() {
  Point2d<T> p;
  p.x[0] = -x[0];
  p.x[1] = -x[1];
  return p;
}
template <class T>
T Point2d<T>::crossp(Point2d<T> p2 ) {/* ?????????? */
  return ( - ( x[0]*p2.x[1] - x[1]*p2.x[0])  );
}
template <class T>
T Point2d<T>::crossp2(Point2d<T> p2 ) {/* ?????????? */
  return ( x[0]*p2.x[1] - x[1]*p2.x[0]  );
}
template <class T>
T Point2d<T>::scalarp(Point2d<T> p2 ) {
  return x[0]*p2.x[0] + x[1]*p2.x[1];
}
template <class T>
T Point2d<T>::projectp(Point2d<T> r0) {
  r0.scalep(r0.len());
  return( ( x[0]*r0.x[0] + x[1]*r0.x[1] ) / r0.len() );
}

template <class T>
bool Point2d<T>::isthesame(Point2d<T> p2, T eps) {
  Point2d<T> pa;
  pa.x[0] = x[0] - p2.x[0];
  pa.x[1] = x[1] - p2.x[1];
  if ( pa.len2() < pow(eps, 2.0) )
    return true;
  
  return false;
}


template <class T>
void Point2d<T>::setvalue(T value) {
  x[0] = value;  
  x[1] = value;
}
template <class T>
void Point2d<T>::setvalue(T value_x, T value_y) {
  x[0] = value_x;  
  x[1] = value_y;
}

template <class T>
void Point2d<T>::rot90() {
  T a=x[0];
  x[0] = -x[1];
  x[1] = a;
}

template <class T>
Point2d<T> Point2d<T>::rotr90() {
  Point2d<T> xx;
  xx.x[0] = -x[1];
  xx.x[1] = x[0];
  return xx;
}

template <class T>
void Point2d<T>::minP(Point2d<T> p) {
  x[0] = min(x[0], p.x[0]);
  x[1] = min(x[1], p.x[1]);
}
template <class T>
void Point2d<T>::minP(Point2d<T> p, int i) {
  if ( i==0 ) {
    x[0] = p.x[0];
    x[1] = p.x[1];
  }
  else {
    x[0] = min(x[0], p.x[0]);
    x[1] = min(x[1], p.x[1]);
  }
}


#endif

