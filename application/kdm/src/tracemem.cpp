/***************************************************************************
                          tracemem.cpp  -  description
                             -------------------
    begin                : Fri Feb 17 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "tracemem.h"

TraceMem::TraceMem(){
  Trace = NULL;

  frac_array = NULL;

  filter20 = NULL;
  filter40 = NULL;
  filter60 = NULL; 
  filter80 = NULL;
  filter_unlimited = NULL;

  bandpass_filter = NULL;

  fftarray = NULL;
  fftarray_filtered = NULL;
  fftarray20 = NULL;
  fftarray40 = NULL;
  fftarray60 = NULL;
  fftarray80 = NULL;
}

TraceMem::~TraceMem(){
    if ( Trace != NULL )
	delete[] Trace;
    if (frac_array != NULL)
    {
	delete[] frac_array;
    }
    if (filter_unlimited != NULL)
    {
	delete[] filter20;
	delete[] filter40;
	delete[] filter60;
	delete[] filter80;
	delete[] filter_unlimited;
    }
    if (fftarray != NULL)
    {
	delete[] fftarray;
	delete[] fftarray_filtered;
	delete[] fftarray20;
	delete[] fftarray40;
	delete[] fftarray60;
	delete[] fftarray80;
    }
    if (bandpass_filter != NULL)
	delete[] bandpass_filter;

}

void TraceMem::Init(TraceFileHandler& TFHandler, const MigrationJob& Job)
{
  if (TFHandler.Next())
    std::cerr << "ERROR in TraceMem: End of TraceFile has been reached!\n";;
  Nt = TFHandler.getNt();
  dtbin = TFHandler.getdt();


  T0 = TFHandler.getT0();
  Tmax = T0 + (Nt-2)*dtbin;


  UTM sx_UTM, sy_UTM;
  UTM gx_UTM, gy_UTM;

  Acq_geometry<float> Geom(Job.geom);
  
  Geom.WORLDxy_to_MODxy(TFHandler.getsx().v, TFHandler.getsy().v, &sx.v, &sy.v); 
  Geom.WORLDxy_to_MODxy(TFHandler.getgx().v, TFHandler.getgy().v, &gx.v, &gy.v); 

  //std::cout << "Nt = " << Nt << ", T0 = " << T0 << ", dtbin = " << dtbin << std::endl;
  if ( Trace == NULL)
  {
      Trace = new float[5*Nt];

      FilteredTraces = new float*[5];
      FilteredTraces[0] = &Trace[1*Nt];
      FilteredTraces[1] = &Trace[2*Nt];
      FilteredTraces[2] = &Trace[3*Nt];
      FilteredTraces[3] = &Trace[4*Nt];
      FilteredTraces[4] = &Trace[0];
  }

  if (Trace == NULL)
    std::cerr << "ERROR in TraceMem: Could not allocate Memory for " << Nt << " floats.\n";

  Frequmin = 20.0f;
  dFrequ = 10.0f;

  TFHandler.ReadTraceData(Trace);
  
  SincInt.init(dtbin);

  PrepareData(Job);
  //Dump();
}

TraceMem::TraceMem(TraceMemHandler& TFHandler)
{
  if (TFHandler.Next())
    std::cerr << "ERROR in TraceMem: End of TraceFile has been reached!\n";;
  Nt = TFHandler.getNt();
  dtbin = TFHandler.getdt();


  T0 = TFHandler.getT0();
  Tmax = T0 + (Nt-2)*dtbin;


  //std::cout << "Nt = " << Nt << ", T0 = " << T0 << ", dtbin = " << dtbin << std::endl;
  Trace = new float[5*Nt];
  if (Trace == NULL)
    std::cerr << "ERROR in TraceMem: Could not allocate Memory for " << Nt << " floats.\n";

  FilteredTraces = new float*[5];
  FilteredTraces[0] = &Trace[1*Nt];
  FilteredTraces[1] = &Trace[2*Nt];
  FilteredTraces[2] = &Trace[3*Nt];
  FilteredTraces[3] = &Trace[4*Nt];
  FilteredTraces[4] = &Trace[0];

  TFHandler.ReadTraceData(Trace);
  //Dump();
}

float TraceMem::GetData(const int& it) const
{
  if ((it >= Nt) || (it < 0))
    return 0;
  else
    return Trace[it];
}

float TraceMem::GetData(const int& it, const float Frequ) const
{
  if ((it >= Nt) || (it < 0))
    return 0;
  else
  {
      unsigned int FilterIndex = std::max(std::min((int)((Frequ-Frequmin)/dFrequ), ( int) 4), 0);
      return FilteredTraces[FilterIndex][it];
  }
}

float TraceMem::GetData(const float& t) const
{
  int it = static_cast<int> ((t-T0)/dtbin);
  float Data;
//  std::cout << "t = " << t << ", it = " << it << std::endl;
  if (it >= (Nt-1))
    Data = 0;
  else
  {
    const float t0 = Trace[it];
    const float t1 = Trace[it+1];
    const float d = (t-(T0+it*dtbin))/dtbin;
    Data = d * t1 + (1-d) * t0;
  }

  const float sign = 1.0f;
  float Amp = sign * SincInt.interp_sinc(&Trace[0], Nt, 0, t, 0);

  return Amp;
}

void TraceMem::PrepareData(const MigrationJob& Job)
{
    // Remove direct wave
     {
 	const float t_direct_wave = sqrt((sx-gx)*(sx-gx) + (sy-gy)*(sy-gy))/1500.0f;
 	for (int it = 0; it < Nt; it++)
 	{
 	    const float t = T0 + it*dtbin;
 	    if ( t < (t_direct_wave + 10*dtbin))
 		Trace[it] = Trace[it] * std::max(0.0, 1.0 - (t_direct_wave + 10*dtbin - t)/(6*dtbin));
 	}
     }
    const int Nfft = getptz(Nt);

    if (frac_array == NULL)
    {
	frac_array = new float[Nfft];
	for(int iarray = 0; iarray < Nfft; iarray++)
	{
	    float omega = 2*2*3.14159*iarray/(dtbin*Nfft);
	    if (iarray >= Nfft/2)
		omega =  0;//-2*3.14159*(Nfft - iarray - 1)/(0.004*Nfft);
	    frac_array[iarray] = omega;
	}
    }

    if (bandpass_filter == NULL)
    {
	bandpass_filter = new float[Nfft];
	if ( (Job.frequ1 > 0)
	     && (Job.frequ2 > 0)
	     && (Job.frequ3 > 0)
	     && (Job.frequ4 > 0))
	{
	    for (int iarray = 0; iarray < Nfft; iarray++)
	    { 
		float frequ = iarray/(dtbin*Nfft);
		bandpass_filter [iarray] = 1.0f;
		if ( (frequ < Job.frequ1) || (frequ >= Job.frequ4))
		    bandpass_filter[iarray] = 0;
		else
		    if ( (frequ >= Job.frequ1) && (frequ < Job.frequ2))
		    {
			const float val = (frequ - Job.frequ1)/(Job.frequ2-Job.frequ1);
			bandpass_filter [iarray] =  (sin((2*val-1)*_PI/2) + 1)*(sin((2*val-1)*_PI/2) + 1)/4;
		    }
		    else
			if ( (frequ >= Job.frequ3) && (frequ < Job.frequ4))
			{
			    const float val = (Job.frequ4 - frequ)/(Job.frequ4-Job.frequ3);
			    bandpass_filter [iarray] =  (sin((2*val-1)*_PI/2) + 1)*(sin((2*val-1)*_PI/2) + 1)/4;
			}
	    }
	}
	else
	{
	    for (int iarray = 0; iarray < Nfft; iarray++)
	    { 
		bandpass_filter[iarray] = 1.0f;
	    }
	}
    }

    if (filter_unlimited == NULL)
    {
	filter20 = new float[Nfft];
	filter40 = new float[Nfft];
	filter60 = new float[Nfft];
	filter80 = new float[Nfft];
	filter_unlimited = new float[Nfft];
    
	for (int iarray = 0; iarray < Nfft; iarray++)
	{ 
	    float frequ = iarray/(dtbin*Nfft);
	    
	    const float frequ20 = Frequmin;
	    const float frequ40 = Frequmin + 1*dFrequ;
	    const float frequ60 = Frequmin + 2*dFrequ;
	    const float frequ80 = Frequmin + 3*dFrequ;
	    
	    filter20 [iarray] = 1.0f;
	    filter40 [iarray] = 1.0f;
	    filter60 [iarray] = 1.0f;
	    filter80 [iarray] = 1.0f;
	    filter_unlimited [iarray] = 1.0f;
	    

	    if (Job.anti_aliasing)
	    {
		if ( frequ >= frequ20 )
		    filter20[iarray] = 0;
		else
		    if ( (frequ >= frequ20-10.0f) && (frequ < frequ20))
		    {
			const float val = (frequ20 - frequ)/(10.0f);
			filter20 [iarray] =  (sin((2*val-1)*_PI/2) + 1)*(sin((2*val-1)*_PI/2) + 1)/4;
		    }
		
		if ( frequ >= frequ40 )
		    filter40[iarray] = 0;
		else
		    if ( (frequ >= frequ40-10.0f) && (frequ < frequ40))
		    {
			const float val = (frequ40 - frequ)/(10.0f);
			filter40 [iarray] =  (sin((2*val-1)*_PI/2) + 1)*(sin((2*val-1)*_PI/2) + 1)/4;
		    }
		
		if ( frequ >= frequ60 )
		    filter60[iarray] = 0;
		else
		    if ( (frequ >= frequ60-10.0f) && (frequ < frequ60))
		    {
			const float val = (frequ60 - frequ)/(10.0f);
			filter60 [iarray] =  (sin((2*val-1)*_PI/2) + 1)*(sin((2*val-1)*_PI/2) + 1)/4;
		    }
		
		if ( frequ >= frequ80 )
		    filter80[iarray] = 0;
		else
		    if ( (frequ >= frequ80-10.0f) && (frequ < frequ80))
		    {
			const float val = (frequ80 - frequ)/(10.0f);
			filter80 [iarray] =  (sin((2*val-1)*_PI/2) + 1)*(sin((2*val-1)*_PI/2) + 1)/4;
		    }
	    }
	    
	//std::cout << iarray << " " << frequ << " " << filter20[iarray] << " " << filter40[iarray] << " " << filter60[iarray] << " " << filter80[iarray] << std::endl; 
	}
    }

    if ( fftarray == NULL )
    {
	fftarray = new float[2*Nfft];
	fftarray_filtered = new vector_float[2*Nfft];
	fftarray20 = new float[2*Nfft];
	fftarray40 = new float[2*Nfft];
	fftarray60 = new float[2*Nfft];
	fftarray80 = new float[2*Nfft];
    }

    for ( unsigned int itrace = 0; itrace < 1; itrace++) //TotNumOfTraces; i++) //i < numberoftraces; i++)
    {
	float* DataPtr = Trace;
	float TraceNormSqr = 0;

	/// FFT
	{
	    memset(fftarray, 0, 2*Nfft*sizeof(float));
	    for (int it = 0; it < Nt; it++)
	    {
		fftarray[2*it] = DataPtr[it];
		TraceNormSqr += DataPtr[it]*DataPtr[it];
	    }
	    
	    fft(-1, Nfft, fftarray);

	    if (Job.frac)
	    {
		for (int iarray = 0; iarray < Nfft; iarray++)
		{
		    {
			const float newim = fftarray[2*iarray] * frac_array[iarray] * bandpass_filter[iarray];
			const float newre = -fftarray[2*iarray+1] *frac_array[iarray] * bandpass_filter[iarray];
			#ifdef __SSE__
 			fftarray_filtered[2*iarray] = _mm_setr_ps(newre * filter20[iarray], newre * filter40[iarray], newre * filter60[iarray], newre * filter80[iarray]);
			fftarray_filtered[2*iarray+1] = _mm_setr_ps(newim * filter20[iarray], newim * filter40[iarray], newim * filter60[iarray], newim * filter80[iarray]);
			#endif
			#ifdef __ALTIVEC__
			fftarray_filtered[2*iarray] = (vector_float) {newre * filter20[iarray], newre * filter40[iarray], newre * filter60[iarray], newre * filter80[iarray]};
			fftarray_filtered[2*iarray+1] = (vector_float) {newim * filter20[iarray], newim * filter40[iarray], newim * filter60[iarray], newim * filter80[iarray]};
			#endif
		    }

		    {
			const float newim = fftarray[2*iarray] * frac_array[iarray] * filter_unlimited[iarray] * bandpass_filter[iarray];
			const float newre = -fftarray[2*iarray+1] *frac_array[iarray] * filter_unlimited[iarray] * bandpass_filter[iarray];
			fftarray[2*iarray] = newre;
			fftarray[2*iarray+1] = newim;
		    }
		}
	    }
	    else
	    {
		for (int iarray = 0; iarray < Nfft; iarray++)
		{
		    {
			const float newre = fftarray[2*iarray] * bandpass_filter[iarray];
			const float newim = fftarray[2*iarray+1] * bandpass_filter[iarray];
			#ifdef __SSE__
			fftarray_filtered[2*iarray] = _mm_setr_ps(newre * filter20[iarray], newre * filter40[iarray], newre * filter60[iarray], newre * filter80[iarray]);
			fftarray_filtered[2*iarray+1] = _mm_setr_ps(newim * filter20[iarray], newim * filter40[iarray], newim * filter60[iarray], newim * filter80[iarray]);
			#endif
			#ifdef __ALTIVEC__
			fftarray_filtered[2*iarray] = (vector_float) {newre * filter20[iarray], newre * filter40[iarray], newre * filter60[iarray], newre * filter80[iarray]};
			fftarray_filtered[2*iarray+1] = (vector_float) {newim * filter20[iarray], newim * filter40[iarray], newim * filter60[iarray], newim * filter80[iarray]};
			#endif
		    }

		    {
			const float newre = fftarray[2*iarray];
			const float newim = fftarray[2*iarray+1];
			fftarray[2*iarray] = newre;
			fftarray[2*iarray+1] = newim;
		    }
		}
	    }


// 	    fft(1, Nfft, fftarray20);
// 	    fft(1, Nfft, fftarray40);
// 	    fft(1, Nfft, fftarray60);
// 	    fft(1, Nfft, fftarray80);
	    sse_fft(1, Nfft, fftarray_filtered);
	    fft(1, Nfft, fftarray);
		    
	    for (int it = 0; it < Nt; it++)
	    {
 		FilteredTraces[0][it] = ((float*)(&fftarray_filtered[2*it]))[0]/Nfft;
 		FilteredTraces[1][it] = ((float*)(&fftarray_filtered[2*it]))[1]/Nfft;
 		FilteredTraces[2][it] = ((float*)(&fftarray_filtered[2*it]))[2]/Nfft;
 		FilteredTraces[3][it] = ((float*)(&fftarray_filtered[2*it]))[3]/Nfft;
// 		FilteredTraces[0][it] = fftarray20[2*it]/Nfft;
// 		FilteredTraces[1][it] = fftarray40[2*it]/Nfft;
// 		FilteredTraces[2][it] = fftarray60[2*it]/Nfft;
// 		FilteredTraces[3][it] = fftarray80[2*it]/Nfft;
		FilteredTraces[4][it] = fftarray[2*it]/Nfft;
	    }

	    float TraceNormSqr0 = 0;
	    float TraceNormSqr1 = 0;
	    float TraceNormSqr2 = 0;
	    float TraceNormSqr3 = 0;
	    float TraceNormSqr4 = 0;
	    for (int it = 0; it < Nt; it++)
	    {
		TraceNormSqr0 +=  FilteredTraces[0][it]*FilteredTraces[0][it];
		TraceNormSqr1 +=  FilteredTraces[1][it]*FilteredTraces[1][it];
		TraceNormSqr2 +=  FilteredTraces[2][it]*FilteredTraces[2][it];
		TraceNormSqr3 +=  FilteredTraces[3][it]*FilteredTraces[3][it];
		TraceNormSqr4 +=  FilteredTraces[4][it]*FilteredTraces[4][it];
	    }

	    float TraceNormalizer0 = 0;
	    float TraceNormalizer1 = 0;
	    float TraceNormalizer2 = 0;
	    float TraceNormalizer3 = 0;
	    float TraceNormalizer4 = 0;
	    if (TraceNormSqr0 > 1e-6)
	      TraceNormalizer0 = sqrt(fabs(TraceNormSqr/TraceNormSqr0));
	    if (TraceNormSqr1 > 1e-6)
	      TraceNormalizer1 = sqrt(fabs(TraceNormSqr/TraceNormSqr1));
	    if (TraceNormSqr2 > 1e-6)
	      TraceNormalizer2 = sqrt(fabs(TraceNormSqr/TraceNormSqr2));
	    if (TraceNormSqr3 > 1e-6)
	      TraceNormalizer3 = sqrt(fabs(TraceNormSqr/TraceNormSqr3));
	    if (TraceNormSqr4 > 1e-6)
		TraceNormalizer4 = sqrt(fabs(TraceNormSqr/TraceNormSqr4));

	    for (int it = 0; it < Nt; it++)
	    {
		FilteredTraces[0][it] *= TraceNormalizer0;
		FilteredTraces[1][it] *= TraceNormalizer1;
		FilteredTraces[2][it] *= TraceNormalizer2;
		FilteredTraces[3][it] *= TraceNormalizer3;
		FilteredTraces[4][it] *= TraceNormalizer4;
	    }
	    
	}

    }

//     delete[] fftarray;
//     delete[] fftarray20;
//     delete[] fftarray40;
//     delete[] fftarray60;
//     delete[] fftarray80;
//     delete[] filter20;
//     delete[] filter40;
//     delete[] filter60;
//     delete[] filter80;
//     delete[] filter_unlimited;
//     delete[] frac_array;
}

void TraceMem::Dump() const
{
  for (int i = 0; i < Nt; i++)
    std::cout << i << " : " << Trace[i] << std::endl;
}
