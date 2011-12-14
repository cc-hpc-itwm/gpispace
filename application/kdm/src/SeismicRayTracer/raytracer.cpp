/***************************************************************************
                          raytracer.cpp  -  description
                             -------------------
    begin                : Thu Apr 6 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


// #include "raytracer.h"

template<class TracingOperator_T>
unsigned long RayTracer<TracingOperator_T>::GetMemoryNeeded(const TracingJob& Job) const
{
    unsigned long MemoryNeeded = 0;
    PropModel<VelGridPointBSpline> TestModel;
    MemoryNeeded += TestModel.GetMemoryNeeded(Job.NVel);
    if (Job.IsoMode == VTI)
    {
	MemoryNeeded += 2*TestModel.GetMemoryNeeded(Job.NVel);
    }
    return MemoryNeeded;
}

template<class TracingOperator_T>
int RayTracer<TracingOperator_T>::execute(const TracingJob& Job, ParallelEnvironment* PE)
{
    int ierr = TTAFH.SetDirectory(Job.TTDirName, Job.TTFilePrefix); 
    const int PRank = PE->GetRank();
    const int PSize = PE->GetNodeCount();

#ifdef __ALTIVEC__
    int NumberofThreads = N_THREADS;
#else
    mctpInit();
    int NumberofThreads = std::min(N_THREADS, mctpGetNumberOfCores());
    mctpCleanup();
#endif

    if (NumberofThreads <=0)
	NumberofThreads = N_THREADS;

//     if ((PRank == 0) && (!Job.g_restart))
//     {
// 	TTAFH.WriteXML(Job);
// 	WFPtSrc TestSource (Job.X0Src, 1500.0f, Job.g_DANGLE, 1);
// 	int norays = TestSource.GetNoRays();
// 	ierr = TTAFH.TouchAngSig(norays, Job.NSrc[0], Job.NSrc[1], Job.NSrc[2], Job.N0Src[2]);
//     }
    if (!Job.g_restart)
    {
	if (PRank == 0)
	    TTAFH.WriteXML(Job);
	
	WFPtSrc TestSource (Job.X0Src, 1500.0f, deg2rad(Job.g_DANGLE), deg2rad(Job.RayAperture_deg), 1);
	int norays = TestSource.GetNoRays();
	
	int proc = 0;
	for (int iz = 0; iz < Job.NSrc[2]; iz++)
	{
	    if (proc == PRank)
		ierr = TTAFH.TouchAngSig(norays, Job.NSrc[0], Job.NSrc[1], 1, Job.N0Src[2] + iz);
	    proc = (proc+1)%PSize;
	}
    }
    PE->Barrier();
    PE->ExitOnError(ierr);

    pthread_t thread[NumberofThreads];
    int rc[NumberofThreads];
    thread_parm_t* parm[NumberofThreads];

    int proc = 0;
    TracingJob SubJob[NumberofThreads];
    for (int ithread = 0; ithread < NumberofThreads; ithread++)
    {
	SubJob[ithread] = Job;
	parm[ithread] = new thread_parm_t;
    }

    ProgressBar progress(((Job.NSrc[0]*Job.NSrc[1] + NumberofThreads * PE->GetNodeCount() - 1)/(NumberofThreads * PE->GetNodeCount())) * Job.NSrc[2]);
    int iSrc0 = 0;
    int iSrc1 = 0;
    for (int nxy = 0; nxy < Job.NSrc[0]*Job.NSrc[1]; nxy+=NumberofThreads)
    {
	if (proc == PRank)
	{
	    for (int ithread = 0; ithread < NumberofThreads; ithread++)
	    {
		SubJob[ithread].N0Src[0] = iSrc0  + (iSrc1 + ithread)/Job.NSrc[1] + Job.N0Src[0];
		SubJob[ithread].N0Src[1] = (iSrc1 + ithread) % Job.NSrc[1] + Job.N0Src[1];
// 		SubJob[ithread].N0Src[0] = (nxy + ithread) / Job.NSrc[1] + Job.N0Src[0];
// 		SubJob[ithread].N0Src[1] = (nxy + ithread) % Job.NSrc[1] + Job.N0Src[1];
		SubJob[ithread].N0Src[2] = Job.N0Src[2];
		    
		SubJob[ithread].NSrc[0] = 1;
		SubJob[ithread].NSrc[1] = 1;
		SubJob[ithread].NSrc[2] = Job.NSrc[2];

		parm[ithread]->Object = this;
		parm[ithread]->Job = &SubJob[ithread];
		parm[ithread]->progress = &progress;
		parm[ithread]->tid = ithread;
		parm[ithread]->rank = PRank;
		
		if (SubJob[ithread].N0Src[0] >= Job.N0Src[0] + Job.NSrc[0])
		    parm[ithread]->active = false;
		else
		    parm[ithread]->active = true;

		if (parm[ithread]->active)
		{
		    rc[ithread] = pthread_create(&thread[ithread], NULL, RayTracer<TracingOperator_T>::run_ThreadStartup, (void*) parm[ithread]);
		    if (rc[ithread]) 
			printf("Failed to create a thread.\n");
		}
	    }

	    for (int ithread = 0; ithread < NumberofThreads; ithread++)
	    {
		if (parm[ithread]->active)
		{
		    rc[ithread] = pthread_join(thread[ithread], NULL);
		    if (rc[ithread])
			printf("Failed to join to  thread , rc=%d\n", rc[ithread]);
		}
	    }
	}
	
	for (int ithread = 0; ithread < NumberofThreads; ithread++)
	{
	    iSrc1++;
	    if (iSrc1 >= Job.NSrc[1])
	    {
		iSrc1 = 0;
		iSrc0++;
	    }
	}

	if (++proc == PSize)
	    proc = 0;
    }

    for (int ithread = 0; ithread < NumberofThreads; ithread++)
    {
	delete parm[ithread];
    }

    
    
    PE->Barrier();
    

    return ierr;
}

/** No descriptions */
template<class TracingOperator_T>
void RayTracer<TracingOperator_T>::run(const TracingJob& Job, ProgressBar* progress, const int PRank, const int ith)
{
  int ierr = 0;
  
  //ierr = TTAFH.SetDirectory(Job.TTDirName, Job.TTFilePrefix); 
  Acq_geometry<float> Geom(Job.geom);

  BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);

#ifdef TIMING
  Timer mytimer;
  double t_all = 0;
  double t_tracing = 0;
  double t_outside = 0;
  double t_write = 0;
  double t_initsource = 0;
  double t_initrays = 0;
#endif

  WFPtSrc* Source = NULL;
// Outer Loop over Sources on the SrfcGrid
  for (int iSrc = Job.N0Src[0]; iSrc < Job.N0Src[0] + Job.NSrc[0]; iSrc++)
    for (int jSrc = Job.N0Src[1]; jSrc < Job.N0Src[1] + Job.NSrc[1]; jSrc++)
      for (int kSrc = Job.N0Src[2]; kSrc < Job.N0Src[2] + Job.NSrc[2]; kSrc++)
      {
        // Start of Memory allocation and initialization preamble and problem setup

        // Initialize the source at iSrc, jSrc
        ierr = this->InitSource(Source, Job, iSrc, jSrc, kSrc, ith );
	if (ierr != 0)
	    continue;
        int norays = Source->GetNoRays();
 	if (Job.g_restart && !TTAFH.CheckAngSig(norays, iSrc, jSrc, kSrc, Job.NtotSrc[1]))
	{
	    if ((ith == 0) && (PRank == 0))
		progress->tic();
 	    continue;
	}

#ifdef TIMING
        t_initsource += mytimer.GetTime();
#endif

        // Allocate and Initialize the Memory for the rays and for the ray-front triangulation
	
        rayMem rayArray(Source->GetVWidth()/2);
        InitRays( rayArray, Source);
#ifdef TIMING
        t_initrays += mytimer.GetTime();
#endif
        
        // End of the memory allocation and initialization preamble and problem setup






// Main timestep loop of the ray tracer
        int noraysoutside = 0;

        AngSig* Signals = new AngSig[norays];

        int t_steps = 0;
        while ( (noraysoutside < norays) && (t_steps < Job.g_MAXTSTEP))
          {
            t_steps++;
            //

            ray3D* theserays[4];
            int theseraysindex[4];
            int ntheserays = 0;
            ray3D dummyray = rayArray[0];

            int tmpiray = 0;
            int noraystmp = norays - noraysoutside;
            for (int iray = 0; iray < norays; iray++) 
              {
                ray3D* ray = &(rayArray[iray]);
                if ( ray->history != -1)
                  {
                    theserays[ntheserays] = ray;
                    theseraysindex[ntheserays] = iray;
                    ntheserays++;
                    tmpiray++;

                    if (tmpiray == noraystmp)
                      {
                        for (int itmp = ntheserays; itmp < 4; itmp++)
                          {
                            theserays[itmp] = &dummyray;
                            theseraysindex[itmp] = -1;
                            ntheserays++;
                          } 
                      }

                    if (ntheserays == 4)
                      { 
                        for (int itmp = 0; itmp < 4; itmp++)
                          {
                            this->StepTracer(*theserays[itmp], ith);
                          }
                        //StepTracer(theserays);
#ifdef TIMING
                        t_tracing += mytimer.GetTime();
#endif

                        for (int itmp = 0; itmp < 4; itmp++)
                          {
                            if (theseraysindex[itmp] != -1)
                              {
                                ray3D* ray = theserays[itmp];

                                if (Outside(*ray))
                                  {
                                    ray->history = -1;
                                    noraysoutside++;
                                    // ray has crossed the upper plane
                                    if ( (ray->x[2] >= Job.X1Vol[2]) )
                                      {
//                                      float dist = norm(ray->x - ray->x_old) * (ray->x_old[2] - Job.X1Vol[2])/(ray->x_old[2] - ray->x[2]);
                                        float dist = (ray->x - ray->x_old).norm() * (ray->x_old[2] - Job.X1Vol[2])/(ray->x_old[2] - ray->x[2]);
                                        ray->Restore();
                                        //const float TT = StepTracer.TraceDistance(*ray, dist, t_steps-1, ith);
					const float TT = this->StepTracer.TraceToSurface(*ray, Job.X1Vol[2], t_steps-1, ith);
                                        const float X_MOD = ray->x[0];
                                        const float Y_MOD = ray->x[1];
					float X_UTM, Y_UTM;
					Geom.MODxy_to_WORLDxy(X_MOD, Y_MOD, &X_UTM, &Y_UTM);
                                        const float X = X_UTM;
                                        const float Y = Y_UTM;
                                        const float V = Source->GetVelocity();
//                                         const float V_Inv = norm(ray->p);
                                        const float V_Inv = (ray->p).norm();
                                        const float X_dir = ray->p[0] / V_Inv;
                                        const float Y_dir = ray->p[1] / V_Inv;
                                        const float detQ = ray->detQ;
                                        const int kmah = ray->kmah;
                                        Signals[theseraysindex[itmp]].Store(X, Y, TT, 0, X_dir, Y_dir, V_Inv, detQ, kmah, V);
                                       }
                                  }


                              }
                          }
#ifdef TIMING
                        t_outside += mytimer.GetTime();
#endif
                        ntheserays = 0;
                      }
                  }
              }

            //
          }// End of main timestep loop



//         for (int iray = 0; iray < norays; iray++) 
//           {
//             std::cout << Signals[iray].arrtime << " " << Signals[iray].x << " " << Signals[iray].y << std::endl;
//           }
//             for (int iray = 0; iray < norays; iray++) 
//               {
//                 ray3D* ray = &(rayArray[iray]);
//                 const float TT = Job.g_MAXTSTEP * Job.g_TSTEPSIZE;
//                 const float X = ray->x[0];
//                 const float Y = ray->x[1];
//                 const float Z = ray->x[2];
//                 const float V_Inv = norm(ray->p);
//                 const float X_dir = ray->p[0] / V_Inv;
//                 const float Y_dir = ray->p[1] / V_Inv;
//                 const float detQ = ray->detQ;
//                 const int kmah = ray->kmah;
//                 Signals[iray].Store(X, Y, TT, 0, X_dir, Y_dir, V_Inv, detQ, kmah, Z);
//               }

	  const point3D<float> SrcPosition(Source->GetPosition());
	  const float XSrc_MOD = SrcPosition[0];
	  const float YSrc_MOD = SrcPosition[1];
	  float XSrc_UTM, YSrc_UTM;
	  Geom.MODxy_to_WORLDxy(XSrc_MOD, YSrc_MOD, &XSrc_UTM, &YSrc_UTM);
	  const float ZSrc_UTM = SrcPosition[2];
                           
	  const vti_velocity v_Source(Source->GetVTIVelocity());
	  const float VTI_epsilon = 0.5 * (v_Source.A11/(v_Source.v*v_Source.v) - 1.0f);
	  const float VTI_delta = 0.5 * (v_Source.Ex/(v_Source.v*v_Source.v)) + VTI_epsilon;
	  TTAFH.WriteAngSig(Signals, norays, iSrc, jSrc, kSrc, Job.NtotSrc[1], XSrc_UTM, YSrc_UTM, ZSrc_UTM, v_Source.v, VTI_epsilon, VTI_delta);
#ifdef TIMING
                    t_write += mytimer.GetTime();
#endif

        delete[] Signals;

	if ((ith == 0) && (PRank == 0))
	    progress->tic();

      }// End of loop over Sources

  if (Source != NULL)
      delete Source;

#ifdef TIMING
  std::cout << "Init Source : " << t_initsource << std::endl;
  std::cout << "Init Rays   : " << t_initrays << std::endl;
  std::cout << "Tracing     : " << t_tracing << std::endl;
  std::cout << "Outside     : " << t_outside << std::endl;
  std::cout << "Writing     : " << t_write << std::endl;
#endif

}


template<class TracingOperator_T>
int RayTracer<TracingOperator_T>::InitRays(rayMem& rayArray, WFPtSrc* Source) const
{
  // Allocate and Initialize the Memory for the rays
  int norays = Source->GetNoRays();
  int ierr = rayArray.reserve(norays);
  if ( ierr != 0 )
    
  else
      for (int iray = 0; iray < norays; iray++)
	  rayArray.push_back(Source->GetRay(iray));
  return ierr;
}

template<class TracingOperator_T>
void * RayTracer<TracingOperator_T>::run_ThreadStartup(void *_parm) {
  thread_parm_t* parm = (thread_parm_t*) _parm;

  RayTracer<TracingOperator_T> *tgtObject = parm->Object;
  TracingJob* Job = parm->Job;
  ProgressBar* progress = parm->progress;

  //printf("Running thread object in a new thread\n");
  //tgtObject->migrate_block(ibx, iby, SopheFan, SopheFanFile, BoxVolume, Trace, Result, Job, parm->tid);
  //tgtObject->migrate_block_stack(ibx, iby, SopheFan, SopheFanFile, BoxVolume, Trace, Result, Job, parm->tid);
  tgtObject->run(*Job, progress, parm->rank, parm->tid );
  return NULL;
}
