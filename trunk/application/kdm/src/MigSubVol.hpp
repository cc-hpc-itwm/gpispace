/***************************************************************************
                          MigSubVol.hpp  -  description
*/

/**
    Doxygen Style
    Multi-Line Documentation
**/

/*                           -------------------
    begin                :
    copyright            : (C) 2010 by Daniel Grünewald
    email                : Daniel.Gruenewald@itwm.fraunhofer.de
 ***************************************************************************/


#ifndef MIGSUBVOL_H
#define MIGSUBVOL_H

// user defined includes
#include <MigVol.hpp>

// standard includes


/**
  *@author Daniel Grünewald doxygen stale
  */

class MigSubVol3D : public MigVol3D
{

// public methods
 public:
  // construct the migration subvolume
  // the subvol_id is assumed to vary between
  // 1 and Nsubvol.
  MigSubVol3D(MigVol3D& MigVol,int subvol_id,int Nsubvol);
  MigSubVol3D(MigSubVol3D& MigSubVol,int subvol_id,int Nsubvol);

  ~MigSubVol3D();


  // get the index of the corner of the subvolume relative to the
  // original volume origin
  int getix0();
  int getiy0();
  int getiz0();

  void clear();

  float * getMemPtr();
  void setMemPtr(float * _pSubVolMem, const int &_size);
  int getSizeofSVD(); // Return the required memory size in bytes for
                      // storing the subvolume.

// public attributes
 public:

// private methods
 private:

    MigSubVol3D(const MigSubVol3D& SubVol);

    MigSubVol3D& operator = (const MigSubVol3D& SubVol);

    point3D<float> getSubVolx0(const MigVol3D &_MigVol3D,
			       const int &_subvol_id,
			       const int &_Nsubvol);
/*    point3D<float> getSubVolx0(const MigSubVol3D &_MigVol,
			       const int &_SubVol_id,
			       const int &_NSubVols);*/

    point3D<int>   getSubVolN (const MigVol3D &_MigVol3D,
			       const int &_subvol_id,
			       const int &_Nsubvol);
//     point3D<int>   getSubVolN (const MigSubVol3D &_MigVol3D,
// 			       const int &_subvol_id,
// 			       const int &_Nsubvol);

    point3D<float> getSubVoldx(const MigVol3D &_MigVol3D,
			       const int &_subvol_id,
			       const int &_Nsubvol);

// private attributes
 private:
  int SubVol_id;
  point3D<int> SubVoliX0;
  float * pSubVolMem;

};

#endif

