/***************************************************************************
                          smoothingclass.cpp  -  description

                             -------------------
    begin                : Tue Jan 25 2009
    copyright            : (C) 2009 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


#include "smoothingclass.h"

int SmoothingClass::SmoothLinear(const char* InputFileName, 
				 const char* OutputFileName, 
				 const FILE_MODE PropFileMode,
				 const point3D<int> Ndim, 
				 const point3D<float> dx, 
				 const float SmoothingLengthMeter,
				 const point3D<int> dcoarse)
{
    return Smooth(InputFileName, OutputFileName, PropFileMode,
		  Ndim, dx, 
		  SmoothingLengthMeter, dcoarse,
		  0);
}


int SmoothingClass::SmoothInverse(const char* InputFileName, 
				  const char* OutputFileName, 
				  const FILE_MODE PropFileMode,
				  const point3D<int> Ndim, 
				  const point3D<float> dx, 
				  const float SmoothingLengthMeter,
				  const point3D<int> dcoarse)
{
    return Smooth(InputFileName, OutputFileName, PropFileMode,
		  Ndim, dx, 
		  SmoothingLengthMeter, dcoarse,
		  1);
}

int SmoothingClass::Smooth(const char* InputFileName, 
			   const char* OutputFileName, 
			   const FILE_MODE PropFileMode,
			   const point3D<int> Ndim, 
			   const point3D<float> dx, 
			   const float SmoothingLengthMeter,
			   const point3D<int> dcoarse,
			   const int mode)
{
    int Nx = Ndim[1];
    int Ny = Ndim[0];
    int Nz = Ndim[2];

    int dxcoarse = dcoarse[1];
    int dycoarse = dcoarse[0];
    int dzcoarse = dcoarse[2];

    const unsigned int xSmoothingLength = ( (SmoothingLengthMeter>0.0f) && (dx[1]>0.0f) )?
	static_cast<unsigned int> ( SmoothingLengthMeter / dx[1] + 0.5):0;
    const unsigned int ySmoothingLength = ( (SmoothingLengthMeter>0.0f) && (dx[0]>0.0f) )?
	static_cast<unsigned int> ( SmoothingLengthMeter / dx[0] + 0.5):0;
    const unsigned int zSmoothingLength = ( (SmoothingLengthMeter>0.0f) && (dx[2]>0.0f) )?
	static_cast<unsigned int> ( SmoothingLengthMeter / dx[2] + 0.5):0;

    int ierr = 0;
    SeismicFileReader Input(InputFileName, PropFileMode, Nz, ierr);
    if (ierr != 0)
    {
	
	return -1;
    }

    if ( Nx <= 0)
    {
	
	return -1;
    }
    if ( Ny <= 0)
    {
	
	return -1;
    }
    if ( Nz <= 0)
    {
	
	return -1;
    }

    SegYHeader** Header = new SegYHeader*[Nx];
    float**** buffer = new float***[2];
    buffer[0] = new float**[Nx];
    buffer[1] = new float**[Nx];
    for (int ix = 0; ix < Nx; ix++)
    {
	Header[ix] = new SegYHeader[Ny];
	buffer[0][ix] = new float*[Ny];
	buffer[1][ix] = new float*[Ny];
	for (int iy = 0; iy < Ny; iy++)
        {
	    buffer[0][ix][iy] = new float[Nz];
	    buffer[1][ix][iy] = new float[Nz];
	    if ((buffer[0][ix][iy] == NULL) || (buffer[1][ix][iy] == NULL))
	    {
		
		return -1;
	    }
        }
    }

    const int in = 0;
    const int out = 1;
    
    for (int ix = 0; ix < Nx; ix++)
	for (int iy = 0; iy < Ny; iy++)
	{
	    if ( Input.Read(&Header[ix][iy], buffer[in][ix][iy]) != 0)
	    {
		
		return(-1);
	    }
	}

    
  
    // Decide on job size
#ifdef __ALTIVEC__
    int NumberofThreads = N_THREADS;
#else
    mctpInit();
    int NumberofThreads = std::min(N_THREADS, mctpGetNumberOfCores());
    mctpCleanup();
#endif

    if (NumberofThreads <=0)
	NumberofThreads = N_THREADS;

    pthread_t thread[N_THREADS];
    thread_parm_t thread_parm[N_THREADS];
    for (int ithread = 0; ithread < NumberofThreads; ithread++)
    {
	// thread-parallel in Nx
	const int Nx_per_thread = dxcoarse * (  (Nx/NumberofThreads) / dxcoarse);
	const int Nx_start = ithread * Nx_per_thread;
	const int Nx_this_thread = (ithread == NumberofThreads-1)?(Nx-Nx_start):Nx_per_thread;

	thread_parm[ithread].Nx_start = Nx_start;
	thread_parm[ithread].Nx_end = Nx_start + Nx_this_thread;
	thread_parm[ithread].Ndim = point3D<unsigned int>(Nx, Ny, Nz);
	thread_parm[ithread].dcoarse = point3D<unsigned int>(dxcoarse, dycoarse, dzcoarse);
	thread_parm[ithread].SmoothingLength = point3D<unsigned int>(xSmoothingLength, ySmoothingLength, zSmoothingLength);
	thread_parm[ithread].buffer = buffer;
	thread_parm[ithread].in = in;
	thread_parm[ithread].out = out;
	thread_parm[ithread].mode = mode;
	thread_parm[ithread].tid = ithread;

	const int rc = pthread_create(&thread[ithread], NULL, SmoothingClass::Smooth_Loop_ThreadStartup, (void*) &thread_parm[ithread]);
    }
    for (int ithread = 0; ithread < NumberofThreads; ithread++)
    {
	const int rc = pthread_join(thread[ithread], NULL);
    }

    SeismicFileWriter Output(OutputFileName, PropFileMode, ((Nz+dzcoarse-1)/dzcoarse), Header[0][0].dt * dzcoarse, ierr);
    if (ierr != 0)
    {
	
	return -1;
    }

    
    for (int ix = 0; ix < Nx; ix+=dxcoarse)
	for (int iy = 0; iy < Ny; iy+=dycoarse)
	{
	    Header[ix][iy].ns = (Nz+dzcoarse-1)/dzcoarse;
	    Header[ix][iy].dt = Header[ix][iy].dt * dzcoarse;
	    // set the unassigned part to 0 to eliminate additional information about dimensions
	    memset(Header[ix][iy].unass, 0, 30 * sizeof(short) );
	    if ( Output.Write(&Header[ix][iy], buffer[out][ix][iy]) != 0)
	    {
		
		return -1;
	    }
	}

    
    
    
    
    Input.Close();
    Output.Close();

    for (int ix = 0; ix < Nx; ix++)
    {
	for (int iy = 0; iy < Ny; iy++)
        {
	    delete[] buffer[1][ix][iy];
	    delete[] buffer[0][ix][iy];
        }
	delete[] buffer[1][ix];
	delete[] buffer[0][ix];
	delete[] Header[ix];
    }
    delete[] buffer[1];
    delete[] buffer[0];
    delete[] buffer;
    delete[] Header;

    return 0;
}

void* SmoothingClass::Smooth_Loop_ThreadStartup(void* parmptr)
{
    const thread_parm_t* thread_parm_ptr = (thread_parm_t*) parmptr;
    const int Nx_start = thread_parm_ptr->Nx_start;
    const int Nx_end = thread_parm_ptr->Nx_end;
    const point3D<unsigned int> Ndim = thread_parm_ptr->Ndim;
    const point3D<unsigned int> dcoarse = thread_parm_ptr->dcoarse;
    const point3D<unsigned int> SmoothingLength = thread_parm_ptr->SmoothingLength;
    float**** buffer = thread_parm_ptr->buffer;
    const int in = thread_parm_ptr->in;
    const int out = thread_parm_ptr->out;
    const int tid = thread_parm_ptr->tid;

    switch (thread_parm_ptr->mode)
    {
	case 0:
	    SmoothLinear_Loop(Nx_start, Nx_end,
			      Ndim,
			      dcoarse,
			      SmoothingLength,
			      buffer,
			      in, out,
			      tid);
	    break;
	case 1:
	    SmoothInverse_Loop(Nx_start, Nx_end,
			       Ndim,
			       dcoarse,
			       SmoothingLength,
			       buffer,
			       in, out,
			       tid);
	    break;
    }
    return NULL;
};

void SmoothingClass::SmoothLinear_Loop(const int Nx_start, const int Nx_end,
				       const point3D<unsigned int> Ndim,
				       const point3D<unsigned int> dcoarse,
				       const point3D<unsigned int> SmoothingLength,
				       float**** buffer,
				       const int in, const int out,
				       const int tid)
{
    const int Nx = Ndim[0];
    const int Ny = Ndim[1];
    const int Nz = Ndim[2];

    const int dxcoarse = dcoarse[0];
    const int dycoarse = dcoarse[1];
    const int dzcoarse = dcoarse[2];

    const int xSmoothingLength = SmoothingLength[0];
    const int ySmoothingLength = SmoothingLength[1];
    const int zSmoothingLength = SmoothingLength[2];

    ProgressBar progress( (( (Nx_end - Nx_start) + dxcoarse-1)/dxcoarse ) * (( Ny + dycoarse-1)/dycoarse ));
    for (int ix = Nx_start; ix < Nx_end; ix+=dxcoarse)
    {
	for (int iy = 0; iy < Ny; iy+=dycoarse)
        {
	    for (int iz = 0; iz < Nz; iz+=dzcoarse)
            {
		const int ixa = ((ix - xSmoothingLength)<0)?0:ix - xSmoothingLength;
		const int ixe = (ix + xSmoothingLength + 1)>Nx?Nx:ix + xSmoothingLength + 1;

		const int iya = ((iy - ySmoothingLength)<0)?0:iy - ySmoothingLength;
		const int iye = (iy + ySmoothingLength + 1)>Ny?Ny:iy + ySmoothingLength + 1;

		const int iza = ((iz - zSmoothingLength)<0)?0:iz - zSmoothingLength;
		const int ize = (iz + zSmoothingLength + 1)>Nz?Nz:iz + zSmoothingLength + 1;

		float sum = 0;
		int count = 0;
		for (int iix = ixa; iix < ixe; iix++)
                {
		    for (int iiy = iya; iiy < iye; iiy++)
                    {
			for (int iiz = iza; iiz < ize; iiz++)
                        {
			    sum += buffer[in][iix][iiy][iiz];
			    count++;
                        }
                    }
                }
              
		buffer[out][ix][iy][iz/dzcoarse] = sum/count;
		if (tid == 0)
		    progress.tic();
            }
        }
    }
}

void SmoothingClass::SmoothInverse_Loop(const int Nx_start, const int Nx_end,
					const point3D<unsigned int> Ndim,
					const point3D<unsigned int> dcoarse,
					const point3D<unsigned int> SmoothingLength,
					float**** buffer,
					const int in, const int out,
					const int tid)
{
    const int Nx = Ndim[0];
    const int Ny = Ndim[1];
    const int Nz = Ndim[2];

    const int dxcoarse = dcoarse[0];
    const int dycoarse = dcoarse[1];
    const int dzcoarse = dcoarse[2];

    const int xSmoothingLength = SmoothingLength[0];
    const int ySmoothingLength = SmoothingLength[1];
    const int zSmoothingLength = SmoothingLength[2];

    ProgressBar progress( (( (Nx_end - Nx_start) + dxcoarse-1)/dxcoarse ) * (( Ny + dycoarse-1)/dycoarse ));
    for (int ix = Nx_start; ix < Nx_end; ix+=dxcoarse)
    {
	for (int iy = 0; iy < Ny; iy+=dycoarse)
        {
	    for (int iz = 0; iz < Nz; iz+=dzcoarse)
            {
		const int ixa = ((ix - xSmoothingLength)<0)?0:ix - xSmoothingLength;
		const int ixe = (ix + xSmoothingLength + 1)>Nx?Nx:ix + xSmoothingLength + 1;

		const int iya = ((iy - ySmoothingLength)<0)?0:iy - ySmoothingLength;
		const int iye = (iy + ySmoothingLength + 1)>Ny?Ny:iy + ySmoothingLength + 1;

		const int iza = ((iz - zSmoothingLength)<0)?0:iz - zSmoothingLength;
		const int ize = (iz + zSmoothingLength + 1)>Nz?Nz:iz + zSmoothingLength + 1;

		float sum = 0;
		int count = 0;
		for (int iix = ixa; iix < ixe; iix++)
                {
		    for (int iiy = iya; iiy < iye; iiy++)
                    {
			for (int iiz = iza; iiz < ize; iiz++)
                        {
			    sum += 1.0f/buffer[in][iix][iiy][iiz];
			    count++;
                        }
                    }
                }
              
		buffer[out][ix][iy][iz/dzcoarse] = count/sum;
		if (tid == 0)
		    progress.tic();
            }
        }
    }
}

