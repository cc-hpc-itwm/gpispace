#include <MigSubVol.hpp>

// Construct the MigSubVol
MigSubVol3D::MigSubVol3D(MigVol3D& _MigVol,int _SubVol_id, int _NSubVols)
  : MigVol3D(getSubVolx0(_MigVol,_SubVol_id,_NSubVols),
	     getSubVolN (_MigVol,_SubVol_id,_NSubVols),
	     getSubVoldx(_MigVol,_SubVol_id,_NSubVols))
{
  // Description:
  // Divide the volume into _NSubVols slices along the slowest coordinate
  //

  // Set the subvolume id
  if( (SubVol_id=_SubVol_id)>_NSubVols)
  {

  }

  float * pVolMem;
  // Obtain information about the migration volume
  if( (pVolMem=_MigVol.getMemPtr())==NULL)
  {
    //
  }

  const int VolNx(_MigVol.getNx());

  // Compute the extension of the subvolume
  int tmp1=VolNx / _NSubVols;
  int tmp2=VolNx % _NSubVols;

  const int SubVolNx=( _SubVol_id <= tmp2 ) ? (tmp1+1) : tmp1 ;

  // compute the index, where the  subvolume starts from
  const int Vol_ix=( _SubVol_id -1 )* SubVolNx + ( ( _SubVol_id <= tmp2 ) ? 0 : tmp2 );
  const int Vol_iy=0;
  const int Vol_iz=0;

  const int VolNy =_MigVol.getNy();
  const int VolNz =_MigVol.getNz();

  SubVoliX0[0]=Vol_ix;
  SubVoliX0[1]=Vol_iy;
  SubVoliX0[2]=Vol_iz;

  // Set the pointer to the subvolume memory
  pSubVolMem=pVolMem+( ( Vol_ix*VolNy+Vol_iy )*VolNz+Vol_iz );

  /*std::cout<<"Hi, I am the constructor of MigSubVol3D and my"<<std::endl;
  std::cout<<"MemAddress I am using is given by "<<pSubVolMem<<std::endl;
  std::cout<<"and I was derived from "<<pVolMem<<std::endl<<std::endl;*/

}

// Construct the MigSubVol
MigSubVol3D::MigSubVol3D(MigSubVol3D& _MigVol,int _SubVol_id, int _NSubVols)
  : MigVol3D(getSubVolx0(_MigVol,_SubVol_id,_NSubVols),
	     getSubVolN (_MigVol,_SubVol_id,_NSubVols),
	     getSubVoldx(_MigVol,_SubVol_id,_NSubVols))
{
  // Description:
  // Divide the volume into _NSubVols slices along the slowest coordinate
  //

  // Set the subvolume id
  if( (SubVol_id=_SubVol_id)>_NSubVols)
  {

  }

  float * pVolMem;
  // Obtain information about the migration volume
  if( (pVolMem=_MigVol.getMemPtr())==NULL)
  {
    //
  }

  const int VolNx(_MigVol.getNx());

  // Compute the extension of the subvolume
  int tmp1=VolNx / _NSubVols;
  int tmp2=VolNx % _NSubVols;

  const int SubVolNx=( _SubVol_id <= tmp2 ) ? (tmp1+1) : tmp1 ;

  // compute the index, where the  subvolume starts from
  // relative to the parent index
  const int Vol_ix=( _SubVol_id -1 )* SubVolNx + ( ( _SubVol_id <= tmp2 ) ? 0 : tmp2 );
  const int Vol_iy=0;
  const int Vol_iz=0;

  const int VolNy =_MigVol.getNy();
  const int VolNz =_MigVol.getNz();

  SubVoliX0[0]=Vol_ix+_MigVol.getix0();
  SubVoliX0[1]=Vol_iy+_MigVol.getiy0();
  SubVoliX0[2]=Vol_iz+_MigVol.getiz0();

  // Set the pointer to the subvolume memory
  pSubVolMem=pVolMem+( ( Vol_ix*VolNy+Vol_iy )*VolNz+Vol_iz );

  /*std::cout<<"Hi, I am the constructor of MigSubVol3D and my"<<std::endl;
  std::cout<<"MemAddress I am using is given by "<<pSubVolMem<<std::endl;
  std::cout<<"and I was derived from "<<pVolMem<<std::endl<<std::endl;*/

}


MigSubVol3D::~MigSubVol3D()
{
  /*std::cout<<"Hi, I am the destructor of MigSubVol3D and my"<<std::endl;
  std::cout<<"MemAddress I am using is given by "<<pSubVolMem<<std::endl<<std::endl;*/
  pSubVolMem=NULL;
}

int MigSubVol3D::getix0()
{
  return SubVoliX0[0];
}

int MigSubVol3D::getiy0()
{
  return SubVoliX0[1];
}

int MigSubVol3D::getiz0()
{
  return SubVoliX0[2];
}

void MigSubVol3D::clear()
{

  if(pSubVolMem==NULL)
  {

    // (re-)initialize the subvolume to 0.
  }
  else
  {
    const int SubVolNx(getNx());
    const int SubVolNy(getNy());
    const int SubVolNz(getNz());

    for(int ix=0;ix<SubVolNx;ix++)
    {
      for(int iy=0;iy<SubVolNy;iy++)
      {
        for(int iz=0;iz<SubVolNz;iz++)
        {
          pSubVolMem[(ix*SubVolNy+iy)*SubVolNz+iz]=0.;
        }
      }
    }
  }
}

float * MigSubVol3D::getMemPtr()
{
  //std::cout<<"I am getMemPtr() of MigSubVol3D"<<std::endl;
  return pSubVolMem;
}

void MigSubVol3D::setMemPtr(float * _pSubVolMem, const int &_size)
{
  if(_size<getSizeofSVD())
    std::cerr<<"Size of memory ptr is not sufficient for storage of SubVol3D data !!!"<<std::endl;
  else
    pSubVolMem=_pSubVolMem;
}

int MigSubVol3D::getSizeofSVD()
{
  const int SubVolNx=getNx();
  const int SubVolNy=getNy();
  const int SubVolNz=getNz();

  return ( SubVolNx*SubVolNy*SubVolNz*sizeof(float) );
}

point3D<float> MigSubVol3D::getSubVolx0(const MigVol3D &_MigVol,
					const int &_SubVol_id,
					const int &_NSubVols)
{

  const int VolNx(_MigVol.getNx());

  // Compute the extension of the subvolume
  int tmp1=VolNx / _NSubVols;
  int tmp2=VolNx % _NSubVols;

  const int SubVolNx=( _SubVol_id <= tmp2 ) ? (tmp1+1) : tmp1 ;

  // compute the index, where the  subvolume starts from
  const int Vol_ix=( _SubVol_id -1 )* SubVolNx + ( ( _SubVol_id <= tmp2 ) ? 0 : tmp2 );
  const int Vol_iy=0;
  const int Vol_iz=0;

  // compute the physical point, where the subvolume starts from
  point3D<float> SubVolX0 = _MigVol.GetCoord(Vol_ix, Vol_iy, Vol_iz);

  return SubVolX0;

}

// point3D<float> MigSubVol3D::getSubVolx0(const MigSubVol3D &_MigVol,
// 					const int &_SubVol_id,
// 					const int &_NSubVols)
// {
//
//   const int VolNx(_MigVol.getNx());
//
//   // Compute the extension of the subvolume
//   int tmp1=VolNx / _NSubVols;
//   int tmp2=VolNx % _NSubVols;
//
//   const int SubVolNx=( _SubVol_id <= tmp2 ) ? (tmp1+1) : tmp1 ;
//
//   // compute the index, where the  subvolume starts from
//   const int Vol_ix=( _SubVol_id -1 )* SubVolNx + ( ( _SubVol_id <= tmp2 ) ? 0 : tmp2 );
//   const int Vol_iy=0;
//   const int Vol_iz=0;
//
//   // compute the physical point, where the subvolume starts from
//   point3D<float> SubVolX0 = _MigVol.GetCoord(Vol_ix, Vol_iy, Vol_iz);
//
//   //std::cout<<"MigSubVol3D::getSubVolx0(MigSubVol3D, ...): X0[0]="<<SubVolX0[0]<<std::endl;
//   //std::cout<<"MigSubVol3D::getSubVolx0(MigSubVol3D, ...): X0[1]="<<SubVolX0[1]<<std::endl;
//   //std::cout<<"MigSubVol3D::getSubVolx0(MigSubVol3D, ...): X0[2]="<<SubVolX0[2]<<std::endl;
//
//   return SubVolX0;
//
// }

point3D<int>   MigSubVol3D::getSubVolN (const MigVol3D &_MigVol,
			                const int &_SubVol_id,
			                const int &_NSubVols)
{

  const int VolNx(_MigVol.getNx());
  const int VolNy(_MigVol.getNy());
  const int VolNz(_MigVol.getNz());


  // Compute the extension of the subvolume
  int tmp1=VolNx / _NSubVols;
  int tmp2=VolNx % _NSubVols;

  point3D<int> SubVolN;
  SubVolN[0] = ( _SubVol_id <= tmp2 ) ? (tmp1+1) : tmp1 ; 	// SubVolNx
  SubVolN[1] = VolNy;						// SubVolNy
  SubVolN[2] = VolNz;						// SubVolNz

  /*std::cout<<"MigSubVol3D::getSubVolN(MigVol3D, ...): mtid="<<_SubVol_id<<" VolNx="<<VolNx<<" N[0]="<<SubVolN[0]<<std::endl;
  std::cout<<"MigSubVol3D::getSubVolN(MigVol3D, ...): mtid="<<_SubVol_id<<" VolNy="<<VolNy<<" N[1]="<<SubVolN[1]<<std::endl;
  std::cout<<"MigSubVol3D::getSubVolN(MigVol3D, ...): mtid="<<_SubVol_id<<" VolNz="<<VolNz<<" N[2]="<<SubVolN[2]<<std::endl;*/

  return SubVolN;

}

// point3D<int>   MigSubVol3D::getSubVolN (const MigSubVol3D &_MigVol,
// 			                const int &_SubVol_id,
// 			                const int &_NSubVols)
// {
//
//   const int VolNx(_MigVol.getNx());
//   const int VolNy(_MigVol.getNy());
//   const int VolNz(_MigVol.getNz());
//
//
//   // Compute the extension of the subvolume
//   int tmp1=VolNx / _NSubVols;
//   int tmp2=VolNx % _NSubVols;
//
//   point3D<int> SubVolN;
//   SubVolN[0] = ( _SubVol_id <= tmp2 ) ? (tmp1+1) : tmp1 ; 	// SubVolNx
//   SubVolN[1] = VolNy;						// SubVolNy
//   SubVolN[2] = VolNz;						// SubVolNz
//
//   /*std::cout<<"MigSubVol3D::getSubVolN(MigSubVol3D, ...): mtid="<<_SubVol_id<<" VolNx="<<VolNx<<" N[0]="<<SubVolN[0]<<std::endl;
//   std::cout<<"MigSubVol3D::getSubVolN(MigSubVol3D, ...): mtid="<<_SubVol_id<<" VolNy="<<VolNy<<" N[1]="<<SubVolN[1]<<std::endl;
//   std::cout<<"MigSubVol3D::getSubVolN(MigSubVol3D, ...): mtid="<<_SubVol_id<<" VolNz="<<VolNz<<" N[2]="<<SubVolN[2]<<std::endl;*/
//
//   return SubVolN;
//
// }


point3D<float> MigSubVol3D::getSubVoldx(const MigVol3D &_MigVol,
			                const int &_SubVol_id,
			                const int &_NSubVols)
{
  // set the physical spacing between sublattice points
  point3D<float> SubVoldx(_MigVol.getdx(),_MigVol.getdy(),_MigVol.getdz());

  return SubVoldx;

}
