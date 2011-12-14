/***************************************************************************
                          tracerbase.cpp  -  description
                             -------------------
    begin                : Sun Nov 1 2009
    copyright            : (C) 2009 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


// #include "tracerbase.h"

template<class TracingOperator_T> 
TracerBase<TracingOperator_T>::TracerBase(){
}

template<class TracingOperator_T> 
TracerBase<TracingOperator_T>::~TracerBase(){
}

template<class TracingOperator_T> 
TracerBase<TracingOperator_T>::TracerBase(const TracingJob& Job, int& ierr)
{
    StepTracer.Init(Job.g_TSTEPSIZE);
}

template<class TracingOperator_T>
bool TracerBase<TracingOperator_T>::CheckTTGrid(const TracingJob& Job) const
{
    bool someoutside = false;
    bool alloutside = true;
    point3D<float> FirstPointOutside;
    BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);
    for (int iSrc = 0; iSrc < Job.NSrc[0]; iSrc++)
    {
	for (int jSrc = 0; jSrc < Job.NSrc[1]; jSrc++)
	{
	    for (int kSrc = 0; kSrc < Job.NSrc[2]; kSrc++)
	    {
		point3D<float> SrcPoint(Job.X0Src[0] + iSrc*Job.dxSrc[0], 
					Job.X0Src[1] + jSrc*Job.dxSrc[1], 
					Job.X0Src[2] + kSrc*Job.dxSrc[2]);
		if (Outside(SrcPoint))
		{
		    if (someoutside == false)
			FirstPointOutside = SrcPoint;
		    someoutside = true;
		}
		else
		    alloutside = false;
	    }
	}
    }
    if (alloutside)
    {
	Acq_geometry<float> Geom(Job.geom);
	float X_UTM, Y_UTM;
	Geom.MODxy_to_WORLDxy(FirstPointOutside[0], FirstPointOutside[1], &X_UTM, &Y_UTM);
	
	
	
	
	
	
	
    }
    else if (someoutside)
    {
	Acq_geometry<float> Geom(Job.geom);
	float X_UTM, Y_UTM;
	Geom.MODxy_to_WORLDxy(FirstPointOutside[0], FirstPointOutside[1], &X_UTM, &Y_UTM);
	
	
	
	
	
	
	
	
    }
    return !alloutside;
}





template<class TracingOperator_T>
int TracerBase<TracingOperator_T>::InitSource(WFPtSrc*& Source, const TracingJob& Job, const int& iSrc, const int& jSrc , const int& kSrc, const int ith )
{
  if (Source != NULL)
    {
      delete Source;
      Source = NULL;
    }

  // Initialize the Source at SrcPoint with an 
  // initial ray velocity given by the velocity model
  point3D<float> SrcPoint(Job.X0Src[0] + iSrc*Job.dxSrc[0], Job.X0Src[1] + jSrc*Job.dxSrc[1], Job.X0Src[2] + kSrc*Job.dxSrc[2]);


  if (Job.SrcElev)
    {
      std::ifstream InputFile(Job.SrcElevFileName, std::ios::binary);
      if (InputFile.fail())
        
      InputFile.seekg( iSrc*(sizeof(SegYHeader) + Job.NtotSrc[1]*sizeof(float)) + sizeof(SegYHeader) + jSrc*sizeof(float), std::ios::beg);
      float zevel;
      InputFile.read((char*)&zevel, sizeof(float));
      if (InputFile.fail())
        
      InputFile.close();
      SrcPoint[2] = zevel;
    }

  Acq_geometry<float> Geom(Job.geom);
  float X_UTM, Y_UTM;
  Geom.MODxy_to_WORLDxy(SrcPoint[0], SrcPoint[1], &X_UTM, &Y_UTM);
  
  
  

  BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);
  if (Outside(SrcPoint))
    {
      
      return -1;
    }

  VREPR_T velocity;
  this->StepTracer.GetVelocityAt(velocity, SrcPoint, ith);

  if ( Job.RunMode == RAYTRACER )
    Source = new  WFPtSrc(SrcPoint, velocity, deg2rad(Job.g_DANGLE), deg2rad(Job.RayAperture_deg), 1);
  else 
    if ( Job.RunMode == WAVEFRONTTRACER)
      {
	Source = new WFPtSrc(SrcPoint, velocity, deg2rad(Job.g_InitAngle), point3D<float>(0,0,-1), deg2rad(Job.RayAperture_deg), 0);
	
	if (Source->GetNoTriangles() == 0)
	  {
	    std::cerr << "FATAL ERROR: There are no triangles within the aperture " << Job.RayAperture_deg.v << "!\n";
	    exit(1);
	  }
      }

  return 0;
}
