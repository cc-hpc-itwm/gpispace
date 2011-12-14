#include <TraceData.hpp>

TraceData::TraceData()
{
  pTraceData = NULL;

  Trace = NULL;

  FilteredTraces = NULL;

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

  TraceMemRqst=true;
}

TraceData::TraceData(volatile char * _pTraceData, const int &_Nt)
{
  pTraceData=_pTraceData;

  Nt=_Nt;

  //if(Nt!=_Nt)
  //  std::cerr<<"TraceData: mismatch of Nt in char and _Nt"<<std::endl;

  Trace = getTraceFromChar();

  FilteredTraces = new float*[5];
  FilteredTraces[0] = &Trace[1*Nt];
  FilteredTraces[1] = &Trace[2*Nt];
  FilteredTraces[2] = &Trace[3*Nt];
  FilteredTraces[3] = &Trace[4*Nt];
  FilteredTraces[4] = &Trace[0];

  Frequmin = 20.0f;
  dFrequ = 10.0f;

  dtbin	= getdtFromChar();
  T0	= getT0FromChar();
  Tmax	= getTmaxFromChar();
  sx.v	= getsxFromChar();
  sy.v  = getsyFromChar();
  gx.v	= getgxFromChar();
  gy.v 	= getgyFromChar();

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

  TraceMemRqst=false;

  //SincInt.init(dtbin);

}

// TraceData::~TraceData()
// {
//   pTraceData=NULL;
// }


TraceData::~TraceData(){
    if(TraceMemRqst)
    {
      if ( pTraceData != NULL )
      {
	delete[] pTraceData;
      }
    }

    if(FilteredTraces!=NULL)
      delete[] FilteredTraces;

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

void TraceData::SetNewCharPtr(volatile char *_pTraceData,const int &_Nt)
{
  pTraceData=_pTraceData;

  //if(Nt!=_Nt)
  //  std::cerr<<"TraceData: mismatch of Nt in char and _Nt"<<std::endl;

  Trace = getTraceFromChar();

  if(FilteredTraces!=NULL)
  {
    if(Nt!=_Nt)
      std::cerr<<"TraceData: mismatch of Nt in char and _Nt"<<std::endl;
  }
  else
  {
    FilteredTraces = new float*[5];
  }

  Nt=_Nt;

  //getMemVarsFromChar();

  FilteredTraces[0] = &Trace[1*Nt];
  FilteredTraces[1] = &Trace[2*Nt];
  FilteredTraces[2] = &Trace[3*Nt];
  FilteredTraces[3] = &Trace[4*Nt];
  FilteredTraces[4] = &Trace[0];

  Frequmin = 20.0f;
  dFrequ = 10.0f;

  dtbin	= getdtFromChar();
  T0	= getT0FromChar();
  Tmax	= getTmaxFromChar();
  sx.v	= getsxFromChar();
  sy.v  = getsyFromChar();
  gx.v	= getgxFromChar();
  gy.v 	= getgyFromChar();

  TraceMemRqst=false;

  //SincInt.init(dtbin);
}

volatile char * TraceData::getDataPtr()
{
  return pTraceData;
}

void TraceData::LoadFromDisk(TraceFileHandler& TFHandler, const MigrationJob& Job)
{
  if (TFHandler.Next())
    std::cerr << "ERROR in TraceData: End of TraceFile has been reached!\n";


  const int Nt_tmp = TFHandler.getNt();
  dtbin = TFHandler.getdt();


  T0 = TFHandler.getT0();
  Tmax = T0 + (Nt_tmp-2)*dtbin;


  UTM sx_UTM, sy_UTM;
  UTM gx_UTM, gy_UTM;

  Acq_geometry<float> Geom(Job.geom);

  Geom.WORLDxy_to_MODxy(TFHandler.getsx().v, TFHandler.getsy().v, &sx.v, &sy.v);
  Geom.WORLDxy_to_MODxy(TFHandler.getgx().v, TFHandler.getgy().v, &gx.v, &gy.v);

  //std::cout << "Nt = " << Nt << ", T0 = " << T0 << ", dtbin = " << dtbin << std::endl;
  if ( pTraceData == NULL)
  {
      pTraceData = new char[(7+5*Nt_tmp)*sizeof(float)+sizeof(int)];
      //Trace = new float[5*Nt];
      Trace=getTraceFromChar();

      FilteredTraces = new float*[5];
      FilteredTraces[0] = &Trace[1*Nt_tmp];
      FilteredTraces[1] = &Trace[2*Nt_tmp];
      FilteredTraces[2] = &Trace[3*Nt_tmp];
      FilteredTraces[3] = &Trace[4*Nt_tmp];
      FilteredTraces[4] = &Trace[0];
  } else
  {
    if(Nt_tmp!=Nt)
    {
      if(TraceMemRqst)
      {
        pTraceData = new char[(7+5*Nt)*sizeof(float)+sizeof(int)];
        //Trace = new float[5*Nt];
        Trace=getTraceFromChar();

        FilteredTraces = new float*[5];
        FilteredTraces[0] = &Trace[1*Nt];
        FilteredTraces[1] = &Trace[2*Nt];
        FilteredTraces[2] = &Trace[3*Nt];
        FilteredTraces[3] = &Trace[4*Nt];
        FilteredTraces[4] = &Trace[0];
      }
      else
      {
        std::cerr<<"TraceData: pTraceData does not correspond to the required data size!!!"<<std::endl;
        std::cerr<<"           Nt ="<<Nt<<" Nt(data)="<<Nt_tmp<<std::endl;
      }
    } else
    {
      //      std::cout<<"Everything is Ok"<<std::endl;
      //std::cout<<"Trace="<<Trace<<std::endl;
      //std::cout<<"pTraceData="<<(void *)pTraceData<<std::endl;
    }
  }

  Nt=Nt_tmp;

  if (Trace == NULL)
    std::cerr << "ERROR in TraceData: Could not allocate Memory for " << Nt << " floats.\n";

  setNtToChar(Nt);
  setdtToChar(dtbin);
  setT0ToChar(T0);
  setTmaxToChar(Tmax);
  setsxToChar(sx.v);
  setsyToChar(sy.v);
  setgxToChar(gx.v);
  setgyToChar(gy.v);

  Frequmin = 20.0f;
  dFrequ = 10.0f;

  TFHandler.ReadTraceData(Trace);

  //SincInt.init(dtbin);
  //std::cout<<"Dumping Trace Data before PrepareData"<<std::endl;
  //Dump();

  PrepareData(Job);
  //std::cout<<"Dumping Trace Data after PrepareData"<<std::endl;
  //Dump();
}

// TraceData::TraceData(TraceMemHandler& TFHandler)
// {
//   if (TFHandler.Next())
//     std::cerr << "ERROR in TraceData: End of TraceFile has been reached!\n";;
//   Nt = TFHandler.getNt();
//   dtbin = TFHandler.getdt();
//
//
//   T0 = TFHandler.getT0();
//   Tmax = T0 + (Nt-2)*dtbin;
//
//
//   //std::cout << "Nt = " << Nt << ", T0 = " << T0 << ", dtbin = " << dtbin << std::endl;
//   Trace = new float[5*Nt];
//   if (Trace == NULL)
//     std::cerr << "ERROR in TraceData: Could not allocate Memory for " << Nt << " floats.\n";
//
//   FilteredTraces = new float*[5];
//   FilteredTraces[0] = &Trace[1*Nt];
//   FilteredTraces[1] = &Trace[2*Nt];
//   FilteredTraces[2] = &Trace[3*Nt];
//   FilteredTraces[3] = &Trace[4*Nt];
//   FilteredTraces[4] = &Trace[0];
//
//   TFHandler.ReadTraceData(Trace);
//   //Dump();
// }

float TraceData::GetData(const int& it) const
{

  if ((it >= Nt) || (it < 0))
    return 0;
  else
    return Trace[it];
}

float TraceData::GetData(const int& it, const float Frequ) const
{

  if ((it >= Nt) || (it < 0))
    return 0;
  else
  {
      unsigned int FilterIndex = std::max(std::min((int)((Frequ-Frequmin)/dFrequ), ( int) 4), 0);
      return FilteredTraces[FilterIndex][it];
  }
}

float TraceData::GetData(const float& t,SincInterpolator &_SincInt) const
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
  float Amp = sign * _SincInt.interp_sinc(&Trace[0], Nt, 0, t, 0);

  return Amp;
}

void TraceData::PrepareData(const MigrationJob& Job)
{
    // Remove direct wave
     {
 	const float t_direct_wave = sqrt((sx-gx)*(sx-gx) + (sy-gy)*(sy-gy))/1500.0f;
 	for (int it = 0; it < Nt; it++)
 	{
 	    const float t = T0 + ((float)it)*dtbin;
 	    if ( t < (t_direct_wave + 10.*dtbin))
 		Trace[it] = Trace[it] * std::max(0.0, 1.0 - (t_direct_wave + 10*dtbin - t)/(6*dtbin));
 	}
     }
    const int Nfft = getptz(Nt);

    //std::cout<<"TraceData::PrepareData: frac_array="<<frac_array<<std::endl;

    if (frac_array == NULL)
    {
	frac_array = new float[Nfft];
	for(int iarray = 0; iarray < Nfft; iarray++)
	{
	    float omega = 2.*2.*3.14159*((float)iarray)/(dtbin*((float)Nfft));
	    if (iarray >= Nfft/2)
		omega =  0.;//-2*3.14159*(Nfft - iarray - 1)/(0.004*Nfft);
	    frac_array[iarray] = omega;
	}
    }

    //std::cout<<"TraceData::PrepareData: bandpass_filter="<<bandpass_filter<<std::endl;

    if (bandpass_filter == NULL)
    {
	bandpass_filter = new float[Nfft];
// 	if ( (Job.frequ1 > 0)
// 	     && (Job.frequ2 > 0)
// 	     && (Job.frequ3 > 0)
// 	     && (Job.frequ4 > 0))
// 	{
// 	    for (int iarray = 0; iarray < Nfft; iarray++)
// 	    {
// 		float frequ = ((float)iarray)/(dtbin*((float)Nfft));
// 		bandpass_filter [iarray] = 1.0f;
// 		if ( (frequ < Job.frequ1) || (frequ >= Job.frequ4))
// 		    bandpass_filter[iarray] = 0.;
// 		else
// 		    if ( (frequ >= Job.frequ1) && (frequ < Job.frequ2))
// 		    {
// 			const float val = (frequ - Job.frequ1)/(Job.frequ2-Job.frequ1);
// 			bandpass_filter [iarray] =  (sin((2*val-1)*_PI/2) + 1)*(sin((2*val-1)*_PI/2) + 1)/4;
// 		    }
// 		    else
// 			if ( (frequ >= Job.frequ3) && (frequ < Job.frequ4))
// 			{
// 			    const float val = (Job.frequ4 - frequ)/(Job.frequ4-Job.frequ3);
// 			    bandpass_filter [iarray] =  (sin((2.*val-1.)*_PI/2.) + 1.)*(sin((2.*val-1.)*_PI/2.) + 1.)/4.;
// 			}
// 	    }
// 	}
// 	else
	{
	    for (int iarray = 0; iarray < Nfft; iarray++)
	    {
		bandpass_filter[iarray] = 1.0f;
	    }
	}
    }

    //std::cout<<"TraceData::PrepareData: filter_unlimited="<<filter_unlimited<<std::endl;

    if (filter_unlimited == NULL)
    {
	filter20 = new float[Nfft];
	filter40 = new float[Nfft];
	filter60 = new float[Nfft];
	filter80 = new float[Nfft];
	filter_unlimited = new float[Nfft];

	for (int iarray = 0; iarray < Nfft; iarray++)
	{
	    float frequ = iarray/(dtbin*((float)Nfft));

	    const float frequ20 = Frequmin;
	    const float frequ40 = Frequmin + 1.*dFrequ;
	    const float frequ60 = Frequmin + 2.*dFrequ;
	    const float frequ80 = Frequmin + 3.*dFrequ;

	    filter20 [iarray] = 1.0f;
	    filter40 [iarray] = 1.0f;
	    filter60 [iarray] = 1.0f;
	    filter80 [iarray] = 1.0f;
	    filter_unlimited [iarray] = 1.0f;


	    if (Job.anti_aliasing)
	    {
		if ( frequ >= frequ20 )
		    filter20[iarray] = 0.;
		else
		    if ( (frequ >= frequ20-10.0f) && (frequ < frequ20))
		    {
			const float val = (frequ20 - frequ)/(10.0f);
			filter20 [iarray] =  (sin((2.*val-1.)*_PI/2.) + 1.)*(sin((2.*val-1.)*_PI/2.) + 1.)/4.;
		    }

		if ( frequ >= frequ40 )
		    filter40[iarray] = 0.;
		else
		    if ( (frequ >= frequ40-10.0f) && (frequ < frequ40))
		    {
			const float val = (frequ40 - frequ)/(10.0f);
			filter40 [iarray] =  (sin((2.*val-1.)*_PI/2.) + 1.)*(sin((2.*val-1.)*_PI/2.) + 1.)/4.;
		    }

		if ( frequ >= frequ60 )
		    filter60[iarray] = 0.;
		else
		    if ( (frequ >= frequ60-10.0f) && (frequ < frequ60))
		    {
			const float val = (frequ60 - frequ)/(10.0f);
			filter60 [iarray] =  (sin((2.*val-1.)*_PI/2.) + 1.)*(sin((2.*val-1.)*_PI/2.) + 1.)/4.;
		    }

		if ( frequ >= frequ80 )
		    filter80[iarray] = 0.;
		else
		    if ( (frequ >= frequ80-10.0f) && (frequ < frequ80))
		    {
			const float val = (frequ80 - frequ)/(10.0f);
			filter80 [iarray] =  (sin((2.*val-1.)*_PI/2.) + 1.)*(sin((2.*val-1.)*_PI/2.) + 1.)/4.;
		    }
	    }

	//std::cout << iarray << " " << frequ << " " << filter20[iarray] << " " << filter40[iarray] << " " << filter60[iarray] << " " << filter80[iarray] << std::endl;
	}
    }

    //std::cout<<"TraceData::PrepareData: fftarray="<<fftarray<<std::endl;

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
	float TraceNormSqr = 0.;

	/// FFT
	{
	    memset((void *)fftarray, 0, 2*Nfft*sizeof(float));
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
 		FilteredTraces[0][it] = ((float*)(&fftarray_filtered[2*it]))[0]/((float)Nfft);
 		FilteredTraces[1][it] = ((float*)(&fftarray_filtered[2*it]))[1]/((float)Nfft);
 		FilteredTraces[2][it] = ((float*)(&fftarray_filtered[2*it]))[2]/((float)Nfft);
 		FilteredTraces[3][it] = ((float*)(&fftarray_filtered[2*it]))[3]/((float)Nfft);
// 		FilteredTraces[0][it] = fftarray20[2*it]/Nfft;
// 		FilteredTraces[1][it] = fftarray40[2*it]/Nfft;
// 		FilteredTraces[2][it] = fftarray60[2*it]/Nfft;
// 		FilteredTraces[3][it] = fftarray80[2*it]/Nfft;
		FilteredTraces[4][it] = fftarray[2*it]/((float)Nfft);
	    }

	    float TraceNormSqr0 = 0.;
	    float TraceNormSqr1 = 0.;
	    float TraceNormSqr2 = 0.;
	    float TraceNormSqr3 = 0.;
	    float TraceNormSqr4 = 0.;
	    for (int it = 0; it < Nt; it++)
	    {
		TraceNormSqr0 +=  FilteredTraces[0][it]*FilteredTraces[0][it];
		TraceNormSqr1 +=  FilteredTraces[1][it]*FilteredTraces[1][it];
		TraceNormSqr2 +=  FilteredTraces[2][it]*FilteredTraces[2][it];
		TraceNormSqr3 +=  FilteredTraces[3][it]*FilteredTraces[3][it];
		TraceNormSqr4 +=  FilteredTraces[4][it]*FilteredTraces[4][it];
	    }

	    float TraceNormalizer0 = 0.;
	    float TraceNormalizer1 = 0.;
	    float TraceNormalizer2 = 0.;
	    float TraceNormalizer3 = 0.;
	    float TraceNormalizer4 = 0.;
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

            /*std::cout<<"TraceData::PrepareData: TraceNormalizer0="<<TraceNormalizer0<<std::endl;
            std::cout<<"TraceData::PrepareData: TraceNormalizer1="<<TraceNormalizer1<<std::endl;
            std::cout<<"TraceData::PrepareData: TraceNormalizer2="<<TraceNormalizer2<<std::endl;
            std::cout<<"TraceData::PrepareData: TraceNormalizer3="<<TraceNormalizer3<<std::endl;
            std::cout<<"TraceData::PrepareData: TraceNormalizer4="<<TraceNormalizer4<<std::endl;*/

	    for (int it = 0; it < Nt; it++)
	    {
		FilteredTraces[0][it] *= TraceNormalizer0;
		FilteredTraces[1][it] *= TraceNormalizer1;
		FilteredTraces[2][it] *= TraceNormalizer2;
		FilteredTraces[3][it] *= TraceNormalizer3;
		FilteredTraces[4][it] *= TraceNormalizer4;
	    }

	}
      //std::cout<<"TraceNormSqr="<<TraceNormSqr<<std::endl;
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

void TraceData::Dump() const
{
  for (int i = 0; i < Nt; i++)
    std::cout << i << " : " << Trace[i] << std::endl;
}

void TraceData::GetData(float* t, float* Amp, SincInterpolator &_SincInt) const
{
      float* Tptr[4];
      for (int icont = 0; icont < 4; icont++)
      {
	  Tptr[icont] = &Trace[0];
      }

      int modus[4];
      for (int icont = 0; icont < 4; icont++)
      {
	  modus[icont] = 0;
      }

      vector_float Amp_sse;
#ifdef __ALTIVEC__
      vector_float t_sse = {t[0], t[1], t[2], t[3]};
      _SincInt.interpolate_sinc_sse(Tptr, Nt, T0, t_sse, modus, &Amp_sse);
#endif
#ifdef __SSE__
      Amp_sse = _SincInt.interpolate_sinc_sse(Tptr, Nt, T0, t, modus);
#endif
      for (int icont = 0; icont < 4; icont++)
      {
	  Amp[icont] = ((float*)(&Amp_sse))[icont];
      }

}

void TraceData::GetFilteredData(float* t, float* Amp, const float Frequ,SincInterpolator &_SincInt) const
{
      float* Tptr[4];
      for (int icont = 0; icont < 4; icont++)
      {
	  unsigned int FilterIndex = std::max(std::min((int)((Frequ-Frequmin)/dFrequ), ( int) 4), 0);
	  Tptr[icont] = FilteredTraces[FilterIndex];
      }

      int modus[4];
      for (int icont = 0; icont < 4; icont++)
      {
	  modus[icont] = 0;
      }

      vector_float Amp_sse;
#ifdef __ALTIVEC__
      vector_float t_sse = {t[0], t[1], t[2], t[3]};
      _SincInt.interpolate_sinc_sse(Tptr, Nt, T0, t_sse, modus, &Amp_sse);
#endif
#ifdef __SSE__
      Amp_sse = _SincInt.interpolate_sinc_sse(Tptr, Nt, T0, t, modus);
#endif

      for (int icont = 0; icont < 4; icont++)
      {
	  Amp[icont] = ((float*)(&Amp_sse))[icont];
      }
}


char* TraceData::getTPtr()
{
      return (char*)Trace;
}

void  TraceData::setNtToChar(const int &Nt)
{
  memcpy((void *)pTraceData,(void *)&Nt,sizeof(int));
}

int   TraceData::getNtFromChar()
{
  int * Nt=(int *)pTraceData;
  return *Nt;
}

void  TraceData::setdtToChar(const float &dtbin)
{
 int offset=sizeof(int);
 memcpy((void *)(pTraceData+offset),(void *)&dtbin,sizeof(float));
}

float TraceData::getdtFromChar()
{
  int offset=sizeof(int);
  float * dtbin=(float *)(pTraceData+offset);
  return *dtbin;
}

void  TraceData::setT0ToChar(float &T0)
{
  int offset=sizeof(int)+1*sizeof(float);
  memcpy((void *)(pTraceData+offset),(void *)&T0,sizeof(float));
}

float TraceData::getT0FromChar()
{
  int offset=sizeof(int)+1*sizeof(float);
  float * T0=(float *)(pTraceData+offset);
  return *T0;
}

void  TraceData::setTmaxToChar(const float &Tmax)
{
  int offset=sizeof(int)+2*sizeof(float);
  memcpy((void *)(pTraceData+offset),(void *)&Tmax,sizeof(float));
}

float TraceData::getTmaxFromChar()
{
  int offset=sizeof(int)+2*sizeof(float);
  float * Tmax=(float *)(pTraceData+offset);
  return *Tmax;
}

void  TraceData::setsxToChar(const float &sx)
{
  int offset=sizeof(int)+3*sizeof(float);
  memcpy((void *)(pTraceData+offset),(void *)&sx,sizeof(float));
}

float TraceData::getsxFromChar()
{
  int offset=sizeof(int)+3*sizeof(float);
  float * sx=(float *)(pTraceData+offset);
  return *sx;
}

void  TraceData::setsyToChar(const float &sy)
{
  int offset=sizeof(int)+4*sizeof(float);
  memcpy((void *)(pTraceData+offset),(void *)&sy,sizeof(float));
}

float TraceData::getsyFromChar()
{
  int offset=sizeof(int)+4*sizeof(float);
  float * sy=(float *)(pTraceData+offset);
  return *sy;
}

void  TraceData::setgxToChar(const float &gx)
{
  int offset=sizeof(int)+5*sizeof(float);
  memcpy((void *)(pTraceData+offset),(void *)&gx,sizeof(float));
}

float TraceData::getgxFromChar()
{
  int offset=sizeof(int)+5*sizeof(float);
  float * gx=(float *)(pTraceData+offset);
  return *gx;
}

void  TraceData::setgyToChar(const float &gy)
{
  int offset=sizeof(int)+6*sizeof(float);
  memcpy((void *)(pTraceData+offset),(void *)&gy,sizeof(float));
}

float TraceData::getgyFromChar()
{
  int offset=sizeof(int)+6*sizeof(float);
  float * gy=(float *)(pTraceData+offset);
  return *gy;
}

// void  TraceData::setFrequmin(const float &Frequmin)
// {
//   int offset=sizeof(int)+7*sizeof(float);
//   memcpy((void *)(pTraceData+offset),(void *)&Frequmin,sizeof(float));
// }
//
// float TraceData::getFrequmin()
// {
//   int offset=sizeof(int)+7*sizeof(float);
//   float Frequmin;
//   memcpy((void *)&Freqmin,(void *)(pTraceData+offset),sizeof(float));
// }
//
// void  TraceData::setFrequ(const float &Freq)
// {
//   int offset=sizeof(int)+8*sizeof(float);
//   memcpy((void *)(pTraceData+offset),(void *)&Freq,sizeof(float));
// }
//
// float TraceData::getFrequ()
// {
//   int offset=sizeof(int)+8*sizeof(float);
//   float Freq;
//   memcpy((void *)&Freq,(void *)(pTraceData+offset),sizeof(float));
// }

float * TraceData::getTraceFromChar()
{
  int offset=sizeof(int)+7*sizeof(float);
  return (float *)(pTraceData+offset);
}

void  TraceData::getMemVarsFromChar()
{
  if(pTraceData!=NULL)
  {
    Nt	= *((int *)pTraceData);
    pTraceData+=sizeof(int);

    dtbin	= *((float *)pTraceData);
    pTraceData+=sizeof(float);

    T0	= *((float *)pTraceData);
    pTraceData+=sizeof(float);

    Tmax	= *((float *)pTraceData);
    pTraceData+=sizeof(float);

    sx.v	= *((float *)pTraceData);
    pTraceData+=sizeof(float);

    sy.v	= *((float *)pTraceData);
    pTraceData+=sizeof(float);

    gx.v	= *((float *)pTraceData);
    pTraceData+=sizeof(float);

    gy.v	= *((float *)pTraceData);
    pTraceData+=sizeof(float);

    Trace	= (float *)pTraceData;
  }

}
