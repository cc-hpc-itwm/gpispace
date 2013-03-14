#include <MigVol.hpp>

MigVol3D::MigVol3D()
{
  pVolMem=NULL;
}

MigVol3D::MigVol3D(const point3D<float>& _x0, const point3D<int>& _N,
		   const point3D<float>& _dx) : grid3D(_x0, _N, _dx)
{
  pVolMem=NULL;
}

MigVol3D::~MigVol3D()
{
  /*std::cout<<"Hi, I am the destructor of MigVol3D and my"<<std::endl;
  std::cout<<"MemAddress I am using is given by "<<pVolMem<<std::endl<<std::endl;*/
  pVolMem=NULL;
}

void MigVol3D::InitVol(float * _pVolMemPtr)
{
  pVolMem=_pVolMemPtr;
  /*std::cout<<"Hi, I am the constructor of MigVol3D and my"<<std::endl;
  std::cout<<"MemAddress I am using is given by "<<pVolMem<<std::endl<<std::endl;*/
}

void MigVol3D::clear()
{
  // (re-)initialize the subvolume to 0.
  const int Nx=getNx();
  const int Ny=getNy();
  const int Nz=getNz();

  if ( pVolMem!=NULL)
  {
    for(int ix=0;ix<Nx;ix++)
    {
     for(int iy=0;iy<Ny;iy++)
     {
      for(int iz=0;iz<Nz;iz++)
      {
	pVolMem[(ix*Ny+iy)*Nz+iz]=0.;
      }
     }
    }
   }

}

float * MigVol3D::getMemPtr()
{
    //std::cout<<"I am getMemPtr() of MigVol3D"<<std::endl;
    return pVolMem;
}
