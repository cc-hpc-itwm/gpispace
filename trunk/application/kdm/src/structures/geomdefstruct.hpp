#ifndef GEOMSTRUCT
#define GEOMSTRUCT

#include <string>
#include "structures/deg.h"

using namespace std;

class geomdefstruct {


public:
  geomdefstruct() {
    first_inline_num = 0;
    first_xline_num = 0;
    n_inlines = 1;
    n_xlines = 1;
  };
  ~geomdefstruct() {};

  short corner3set;

  int n_inlines, n_xlines;

  float d_between_inlines, d_between_xlines;

  int one_inline_num, one_xline_num;

  float x_one, y_one; /* location of intersection of one_inline_num and 
			    one_xline_num */

  DEG angle_rot_deg;  /* angle between inline direction and the horizontal axis
			angle: counterclockwise, horizontal axis: pointing East */
//   string inlinedir;          /* after rotation (x-axis points East): 
// 			    y-axis (inline indices counting positive) points North (N)
// 			    or South (S) */
  int inlinedir;          /* after rotation (x-axis points East): 
			    y-axis (inline indices counting positive) points North (N)
			    or South (S) */

  int first_inline_num, first_xline_num;

  /* corner points defining angle and orientation of geometry */
  float inline_low_xline_low_xy[2];
  float inline_low_xline_higher_xy[2];
  float inline_higher_xline_low_xy[2];


  /* in addition to arbitrary point "one" these two points can be specified
     for automatic calculation of the geometry; point "two" and "three" can be
     arbitrarily positioned with the exception that they should not be located
     along one line */
  int two_inline_num, two_xline_num, three_inline_num, three_xline_num;
  float x_two, y_two, x_three, y_three;

};
#endif
