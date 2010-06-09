/***************************************************************************
                          wavefronttracer.cpp  -  description
                             -------------------
    begin                : Thu Apr 6 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/


// #include "wavefronttracer.h"

template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::execute(const TracingJob& Job, ParallelEnvironment* PE, int& ierr)
{
    ierr = TTFH.SetDirectory(Job.TTDirName, Job.TTFilePrefix);
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

    if (PRank == 0)
	TTFH.WriteXML(Job);


    pthread_t* thread = new pthread_t[NumberofThreads];
    int* rc = new int[NumberofThreads];
    thread_parm_t** parm = new thread_parm_t*[NumberofThreads];

    TracingJob* SubJob = new TracingJob[NumberofThreads];
    for (int ithread = 0; ithread < NumberofThreads; ithread++)
    {
	SubJob[ithread] = Job;
	parm[ithread] = new thread_parm_t;
    }
    
    ProgressBar progress(((Job.NSrc[0]*Job.NSrc[1] + NumberofThreads * PE->GetNodeCount() - 1)/(NumberofThreads * PE->GetNodeCount())) * Job.NSrc[2]);

    for (int ithread = 0; ithread < NumberofThreads; ithread++)
    {
	parm[ithread]->Object = this;
	parm[ithread]->Job = &SubJob[ithread];
	parm[ithread]->progress = &progress;
	parm[ithread]->tid = ithread;
	parm[ithread]->rank = PRank;
	parm[ithread]->active = true;
    }

    int proc = 0;
    int nthread = 0;

    for (int nx = Job.N0Src[0]; nx < Job.N0Src[0]+Job.NSrc[0]; nx++)
        for (int ny = Job.N0Src[1]; ny < Job.N0Src[1]+Job.NSrc[1]; ny++)
	    for (int nz = Job.N0Src[2]; nz < Job.N0Src[2]+Job.NSrc[2]; nz++)
            {
		if (proc == PRank)
                {
		    SubJob[nthread].N0Src[0] = nx;
		    SubJob[nthread].N0Src[1] = ny;
		    SubJob[nthread].N0Src[2] = nz;
                  
		    SubJob[nthread].NSrc[0] = 1;
		    SubJob[nthread].NSrc[1] = 1;
		    SubJob[nthread].NSrc[2] = 1;

		    rc[nthread] = pthread_create(&thread[nthread], NULL, WaveFrontTracer<TracingOperator_T>::run_ThreadStartup, (void*) parm[nthread]);
		    if (rc[nthread]) 
			printf("Failed to create a thread.\n");

		    nthread++;

 		    if (nthread == NumberofThreads)
 		    {
			for (int ithread = 0; ithread < NumberofThreads; ithread++)
			{
			    rc[ithread] = pthread_join(thread[ithread], NULL);
			    if (rc[ithread])
				printf("Failed to join to  thread , rc=%d\n", rc[ithread]);
			}
			nthread = 0;
 		    }
		}
		if (++proc == PSize)
		    proc = 0;
            }

    for (int ithread = 0; ithread < nthread; ithread++)
    {
	rc[ithread] = pthread_join(thread[ithread], NULL);
	if (rc[ithread])
	    printf("Failed to join to  thread , rc=%d\n", rc[ithread]);
    }

    for (int ithread = 0; ithread < NumberofThreads; ithread++)
    {
	delete[] parm[ithread];
    }
    delete[] thread;
    delete[] rc;
    delete[] parm;
    delete[] SubJob;
}

/** Initialization of RcvGrid, shooting from the sources given in TracingJob, and storing the resulting traveltimes to files.  */
template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::run(const TracingJob& Job, ProgressBar* progress, const int PRank, const int ith)
{
  int ierr;

  Acq_geometry<float> Geom(Job.geom);

  ReceiverGrid* RcvGrid = new ReceiverGrid(Job.X0Rcv, Job.NRcv, Job.dxRcv);    
  WFPtSrc* Source = NULL;

  BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);
  ObserverOperator<TracingOperator_T> Observer(&this->StepTracer, ith);

  ReflectionOperator Reflect;

// Outer Loop over Sources on the SrfcGrid
  for (int iSrc = Job.N0Src[0]; iSrc < Job.N0Src[0] + Job.NSrc[0]; iSrc++)
    for (int jSrc = Job.N0Src[1]; jSrc < Job.N0Src[1] + Job.NSrc[1]; jSrc++)
      for (int kSrc = Job.N0Src[2]; kSrc < Job.N0Src[2] + Job.NSrc[2]; kSrc++)
      {
        // Start of Memory allocation and initialization preamble and problem setup

        // Initialize the source at iSrc, jSrc
        ierr = this->InitSource(Source, Job, iSrc, jSrc, kSrc, ith);
	if (ierr != 0)
	    continue;

	tracewf(Job, Source, RcvGrid, Observer, Outside, ith);

#ifdef TIMING
        t_all += mytimer.GetTime();
#endif
#ifdef TIMING
        std::cout << "Tracing of " << rayArray.size() << " rays for " << t_steps << " time steps\n";
        std::cout << "This took " << t_all << " seconds\n";
        std::cout << "The subdivision took " << t_sub << " seconds\n";
#endif

#ifdef DEBUG_CHECK
        Checker.CheckTriangleList(TriangleList);
#endif

        TTFH.WriteRcvSegY(Source->GetPosition(), *RcvGrid, Geom, iSrc, jSrc);

	if ((ith == 0) && (PRank == 0))
	    progress->tic();

      }// End of loop over Sources

  delete RcvGrid;
  if (Source != NULL)
      delete Source;

}

/** Initializing one RcvGrid and shooting from one position. The file-output is only executed if store2file is set. The traveltime tables (including the SegY-Headers) are written to the char*-position mem_ptr. */
template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::run_one(const TracingJob& Job, const int iSrc, const int jSrc, const int kSrc, bool store2file, char* mem_ptr, const int PRank, const int ith)
{
Timer mytimer;

  int ierr;

  Acq_geometry<float> Geom(Job.geom);

  ReceiverGrid* RcvGrid = new ReceiverGrid(Job.X0Rcv, Job.NRcv, Job.dxRcv);
  
  WFPtSrc* Source = NULL;

  BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);
  ObserverOperator<TracingOperator_T> Observer(&this->StepTracer, ith);

  ReflectionOperator Reflect;

  // Initialize the source at iSrc, jSrc
  ierr = this->InitSource(Source, Job, iSrc, jSrc, kSrc, ith );
  if (ierr != 0)  { delete RcvGrid; return;  }

  tracewf(Job, Source, RcvGrid, Observer, Outside, ith);

#ifdef DEBUG_CHECK
  Checker.CheckTriangleList(TriangleList);
#endif


//  if (store2file) 
   if (store2file) TTFH.WriteRcvSegY(Source->GetPosition(), *RcvGrid, Geom, iSrc, jSrc);

  int Nx = RcvGrid->getNx();
  int Ny = RcvGrid->getNy();
  int Nz = RcvGrid->getNz();

  point3D<float> X0 = RcvGrid->getX0();
  point3D<float> dx = RcvGrid->getdx();

  SegYHeader* Header =  (SegYHeader*)&(mem_ptr[0]);
  Header->ns = Nz * N_SIGNALS;
  Header->dt = (int) (dx[2] * 1000);
  Header->sx = static_cast<int>(Source->GetPosition()[0]);
  Header->sy = static_cast<int>(Source->GetPosition()[1]);
  Header->selev = -static_cast<int>(Source->GetPosition()[2]);
  Header->gelev = -static_cast<int>(X0[2]);

  for (int i = 0; i < Nx; i++) for (int j = 0; j < Ny; j++) {
	unsigned long Address = sizeof(SegYHeader) + (i * Ny + j)*Nz*N_SIGNALS*sizeof(float);
	float* buffer = (float*) &(mem_ptr[Address]);  // size: Nz * N_SIGNALS
	for (int k = 0; k < Nz; k++) {
	  Receiver* Rcv = RcvGrid->GetPointAt(i, j, k);
	  buffer[k*N_SIGNALS + 0] = Rcv->get_ttime(buffer[k*N_SIGNALS + 1], buffer[k*N_SIGNALS + 2]);
	}
  }

  delete RcvGrid;
  if (Source != NULL) { delete Source;  Source = NULL; }
}

template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::run_1s(const TracingJob& Job, RecSig* Signals)
{
#ifdef TIMING
  Timer mytimer;
  double t_sub = 0;
  double t_all = 0;
  double t_init = 0;
  double t_loop = 0;
  double t_rays = 0;
  double t_triangle = 0;
  double t_subdiv = 0;
  double t_recv = 0;
  double t_copy = 0;
#endif
  long int NumberOfTracingCalls = 0;
  
  ReceiverGrid* RcvGrid = new ReceiverGrid(Job.X0Rcv, Job.NRcv, Job.dxRcv);
  WFPtSrc* Source = NULL;

  InitSource(Source, Job, 0, 0, 0);

  //ReceiverGrid RcvGrid(Job.X0Rcv, Job.NRcv, Job.dxRcv);
  BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);

  ReceiverGrid::iterator iter = RcvGrid->begin(Job.X0Vol, Job.X1Vol);
  for (; !iter.end(); ++iter)
    {
      Receiver* Rcv = (*iter);
      if (Outside.Aperture(Rcv->pos, Source->pos, Job.RayAperture_deg))
        Rcv->active = false;
      if (Outside.Aperture(Rcv->pos, Source->pos + Job.Offs, Job.RayAperture_deg))
        Rcv->active = false;
    }


  // Re-Initialize the receiver grid
  //  std::cout << "\n\n\nClear the Sub-Surface Receiver Grid      ...  " << std::flush;
  RcvGrid->clear();
  //  std::cout << "ok\n";

  // Initialize rayArray
  //  std::cout << "// Init rayArray\n";
  trirayMem rayArray(Source->GetVWidth()/2);
  triMem TriangleList;
  InitRays( rayArray, TriangleList, Source);
#ifdef OUTPUT_TRIANG
  Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), 0);
#endif


  // Init Refinement Operator
  RefinementOperator<TracingOperator_T>* SubDivide = new RefinementOperator<TracingOperator_T>(Job.g_REF_LEN, &this->StepTracer, Source);
  ObserverOperator<TracingOperator_T> Observer(&this->StepTracer);
  //  std::cout << "Starting the Loop now\n";


#ifdef TIMING  
  t_init = mytimer.GetTime();
#endif
  
  for (int t_steps = 1; t_steps < Job.g_MAXTSTEP; t_steps++)
    {
      std::cout << "t_step " << t_steps << std::endl;
      triMem::iterator iter_end(TriangleList.end());
      for (triMem::iterator iter(TriangleList.begin()); iter != iter_end; ++iter)
        {
          //      std::cout << "Starting Triangle Loop\n" << std::flush;

          Triangle* Tri = &(*iter);
          if (Tri->lifesign != Triangle::DELETED)
            {
              Tri->status = ( Tri->status == false);

#ifdef TIMING
              t_triangle += mytimer.GetTime();
#endif

              // start vector ray loop
              triray3D* rays[4];
              triray3D ray_dummy = *(Tri->points[0]);
              int index = 0;
              for (int i = 0; i < 3; i++)
                {
                  if ( Tri->points[i]->mainray.status != Tri->status )
                    {
                      rays[index] = Tri->points[i];
                      rays[index]->mainray.status = (rays[index]->mainray.status == false);
                      index++;
                    }
                }
              //        std::cout << index << std::endl;
              if (index != 0)
                {
                  if (index < 4)
                    {
                      if (Tri->neighs[0] != NULL)
                        {
                          for (int i = 0; i < 3; i++)
                            {
                              if (Tri->neighs[0]->points[i]->mainray.status != Tri->status)
                                {
                                  rays[index] = Tri->neighs[0]->points[i];
                                  rays[index]->mainray.status = (rays[index]->mainray.status == false);
                                  index++;
                                  break;
                                }
                            }
                        }
                    }
                  if (index < 4)
                    {
                      if (Tri->neighs[1] != NULL)
                        {
                          for (int i = 0; i < 3; i++)
                            {
                              if (Tri->neighs[1]->points[i]->mainray.status != Tri->status )
                                {
                                  rays[index] = Tri->neighs[1]->points[i];
                                  rays[index]->mainray.status = (rays[index]->mainray.status == false);
                                  index++;
                                  break;
                                }
                            }
                        }
                    }
                  if (index < 4)
                    {
                      if (Tri->neighs[2] != NULL)
                        {
                          for (int i = 0; i < 3; i++)
                            {
                              if (Tri->neighs[2]->points[i]->mainray.status != Tri->status )
                                {
                                  rays[index] = Tri->neighs[2]->points[i];
                                  rays[index]->mainray.status = (rays[index]->mainray.status == false);
                                  index++;
                                  break;
                                }
                            }
                        }
                    }

                  if (index <= 4)
                    for (int n = 0; n < index; n++)
                      {
                        this->StepTracer(*rays[n]);
                        NumberOfTracingCalls++;
                      }
//                   else
//                     {
//                       for (int n = index; n < 4; n++)
//                         rays[n] = &ray_dummy;

//                       NumberOfTracingCalls++;
//                       StepTracer(rays);
//                     }
                }

              //        for (int n = 0; n < index; n++)
              //        {
              //          rays[n]->status = (rays[n]->status == false);
              //        }

              for (int i = 0; i < 3; i++)
                {
                  if (Outside(Tri->points[i]->mainray))
                    {
                      SubDivide->DeleteTri(Tri);
                       break;
                    }
                }

              // end vector ray loop

              //      for (int i = 0; i < 3; i++)
              //        {
              //          ray3D* ray = Tri->points[i];
              //
              //          if ( ray->status != Tri->status )
              //          {
              //            StepTracer(*ray);
              //            ray->status = (ray->status == false);
              //          }
              //
              //          if (Outside(*ray))
              //          {
              //            WaveFront.DeleteRay(Tri);
              //            Tri->lifesign = Triangle::DELETED;
              //            break;
              //          }
              //        }

#ifdef TIMING
              t_rays += mytimer.GetTime();
#endif
              int nrcv = 0;
              Receiver* RcvTmp[4];
              //        std::cout << "Starting Receiver Loop\n" << std::flush;
              if ( Tri->lifesign != Triangle::DELETED)
                {
                  ReceiverGrid::iterator iter = RcvGrid->begin(Tri);
                  if (!iter.end())
                    {
                      Observer.Init(Tri);
                      for (; !iter.end(); ++iter)
                        {
                          Receiver* Rcv = (*iter);
                          if ( (Rcv->active) && (Rcv->LastTstep < (t_steps-1)) )
                            if ( Observer.RecvInTube(Tri, Rcv))
                              {
                                RcvTmp[nrcv] = Rcv;
                                nrcv++;
                                if (nrcv == 4)
                                  {
                                    Observer(Tri, RcvTmp, t_steps);
                                    nrcv = 0;
                                  }
                              }
                        }
                      for (int ircv = 0; ircv < nrcv; ircv++)
                        Observer(Tri, RcvTmp[ircv], t_steps);
                    }
                }
              //        std::cout << "Receiver Loop done\n" << std::flush;

#ifdef TIMING
              t_recv += mytimer.GetTime();
#endif
            }
          //      std::cout << "Triangle Loop done\n" << std::flush;


        }

      (*SubDivide)(TriangleList, rayArray, t_steps*Job.g_TSTEPSIZE);
      //    std::cout << "SubDivide done\n" << std::flush;
#ifdef OUTPUT_TRIANG
      if ((t_steps % 1) == 0)
        Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), t_steps/1);
#endif
#ifdef DEBUG_CHECK
      Checker.CheckTriangleList(TriangleList);
#endif
#ifdef TIMING
      t_subdiv = mytimer.GetTime();
#endif

    }



#ifdef DEBUG_CHECK
  Checker.CheckTriangleList(TriangleList);
#endif

  const int Nx = RcvGrid->getNx();
  const int Ny = RcvGrid->getNy();
  const int Nz = RcvGrid->getNz();
  int index = 0;
  for (int iRcv = 0; iRcv < Nx; iRcv++)
    for (int jRcv = 0; jRcv < Ny; jRcv++)
      for (int kRcv = 0; kRcv < Nz; kRcv++)
        {
          Signals[index] = *((RcvGrid->GetPointAt(iRcv, jRcv, kRcv))->GetSig());
          index++;
        }
#ifdef TIMING
  t_copy = mytimer.GetTime();
#endif

  std::cout << "Tracing of " << rayArray.size() << " rays for " << Job.g_MAXTSTEP << " time steps\n";
  std::cout << NumberOfTracingCalls << " calls to the Tracing Operator\n";
#ifdef TIMING
  std::cout << "Tracing of " << rayArray.size() << " rays for " << Job.g_MAXTSTEP << " time steps\n";
  std::cout << "The initialization took " << t_init << " seconds\n";
  std::cout << "The ray loop took " << t_rays << " seconds\n";
  std::cout << "The receiver loop took " << t_recv << " seconds\n";
  std::cout << "The subdivision took " << t_subdiv << " seconds\n";
  std::cout << "The triangle loop took " << t_triangle  << " seconds\n";
  std::cout << "The copying took " << t_copy << " seconds\n";
#endif

  delete SubDivide;
  delete RcvGrid;
  if (Source != NULL)
      delete Source;
}

template<class TracingOperator_T>
float WaveFrontTracer<TracingOperator_T>::run_1s1r(const TracingJob& Job)
{
  //  std::cout << "\nRunning with one Source at " << Job.X0Src << std::endl;
  //  std::cout << "        and one Receiver at " << Job.X0Rcv << std::endl;
  //Output.PrintJob(Job);
  
  if ( (fabs(Job.X0Src[0] - Job.X0Rcv[0]) < 1e-4)
       && (fabs(Job.X0Src[1] - Job.X0Rcv[1]) < 1e-4)
       && (fabs(Job.X0Src[2] - Job.X0Rcv[2]) < 1e-4) )
    return 0;

  WFPtSrc* Source = NULL;
  ReceiverGrid* RcvGrid = new ReceiverGrid(Job.X0Rcv, Job.NRcv, Job.dxRcv);   
  //  ReceiverGrid RcvGrid(Job.X0Rcv, Job.NRcv, Job.dxRcv);

  //  BoundaryOperator Outside(Job.X0Vol+Job.dxVel*2, Job.X1Vol-Job.dxVel*2);
  BoundaryTubeOperator Outside(Job.X0Vol, Job.X1Vol, Job.X0Src, Job.X0Rcv, 100);

  InitSource(Source, Job, 0, 0, 0);
//   point3D<float> SrcPoint = Job.X0Src;
//   point3D<float> Direction = Job.X0Rcv - Job.X0Src;
//   float v = VelModel.GetProperty(SrcPoint);
  
  Receiver Rcv(Job.X0Rcv);
  ObserverOperator<TracingOperator_T> Observer(&this->StepTracer);

  // Initialize rayArray
  trirayMem rayArray(Source->GetVWidth()/2);
  triMem TriangleList;
  InitRays( rayArray, TriangleList, Source);

#ifdef OUTPUT_TRIANG
  Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), 0);
#endif


  // Init Refinement Operator
  RefinementOperator<TracingOperator_T>* SubDivide = new RefinementOperator<TracingOperator_T>(Job.g_REF_LEN, &this->StepTracer, Source);

  //  std::cout << "Starting the Loop now\n";
  Timer mytimer;
  double t_sub = 0;
  double t_all = 0;

  int t_steps=0;
  while ( t_steps < Job.g_MAXTSTEP)
    {
      t_steps++;

      int Tri_Index = 0;

      triMem::iterator iter_end = TriangleList.end();
      for (triMem::iterator iter = TriangleList.begin(); iter != iter_end; ++iter)
        {
          Tri_Index++;
          Triangle* Tri = &(*iter);
          if (Tri->lifesign != Triangle::DELETED)
            {
              Tri->status = ( Tri->status == false);

              //std::cout << "Working on Triangle " << Tri_Index << std::endl;
              for (int i = 0; i < 3; i++)
                {
                  triray3D* ray = Tri->points[i];

                  //std::cout << "Tracing Ray " << i << std::endl;
                  if ( ray->mainray.status != Tri->status )
                    {
                      //                          std::cout << "StepTracer\n";
                      this->StepTracer(*ray);
                      //                          std::cout << "StepTracer done\n";
                      ray->mainray.status = (ray->mainray.status == false);
                    }

                  if (Outside(ray->mainray))
                    {
                      SubDivide->DeleteTri(Tri);
                      break;
                    }
                }

              if ( Tri->lifesign != Triangle::DELETED)
                {
                  if (Rcv.LastTstep < t_steps)
                    {
                      Observer(Tri, &Rcv, t_steps);
                    }
                }
            }
        }

#ifdef OUTPUT_TRIANG
      if ((t_steps % 1) == 0)
        Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), t_steps/1);
#endif

      double t_sub_a = mytimer.GetTime();

      (*SubDivide)(TriangleList, rayArray, t_steps*Job.g_TSTEPSIZE);

      double t_sub_e = mytimer.GetTime();

      t_all += t_sub_a + t_sub_e;
      t_sub += t_sub_e;

    }

  t_all += mytimer.GetTime();

  //  std::cout << "Tracing of " << rayArray.size() << " rays for " << t_steps << " time steps\n";
  //  std::cout << "This took " << t_all << " seconds\n";
  //  std::cout << "The subdivision took " << t_sub << " seconds\n";

#ifdef DEBUG_CHECK
  Checker.CheckTriangleList(TriangleList);
#endif

  delete SubDivide;
  delete RcvGrid;
  if (Source != NULL)
      delete Source;

  RecSig* pSig = Rcv.GetSig();
  //std::cout << pSig->arrtime << std::endl;
  return pSig->GetT();
}

/** No descriptions */
template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::run_rays(const TracingJob& Job)
{
  int ierr;

  WFPtSrc* Source = NULL;
  BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);

  ReflectionOperator Reflect;

  const int NVolx = (int)((Job.X1Vol[0] - Job.X0Vol[0])/25.0);
  const int NVoly = (int)((Job.X1Vol[1] - Job.X0Vol[1])/25.0);

  bool** Binning = new bool*[NVolx];
  for (int ix = 0; ix < NVolx; ix++)
    Binning[ix] = new bool[NVoly];

  float TMax = 5.0;
  float TBin = 0.02;
  int Nt = (int)(TMax/TBin);
  int TimeHits[Nt];
  for (int it = 0; it < Nt; it++)
    TimeHits[it] = 0;

  int HitIn = 0;
  int HitOut = 0;
  int HitOut5000 = 0;

  std::cout << "Surface Area is:\n ";
  std::cout << Job.X0Src[0] + Job.N0Src[0] * Job.dxSrc[0] << " < x < " << Job.X0Src[0] + (Job.N0Src[0] + Job.NSrc[0]) * Job.dxSrc[0] << std::endl;
  std::cout << Job.X0Src[1] + Job.N0Src[1] * Job.dxSrc[1] << " < y < " << Job.X0Src[1] + (Job.N0Src[1] + Job.NSrc[1]) * Job.dxSrc[1] << std::endl;

// Outer Loop over Sources on the SrcGrid
  for (int iSrc = Job.N0Src[0]; iSrc < Job.N0Src[0] + Job.NSrc[0]; iSrc++)
    for (int jSrc = Job.N0Src[1]; jSrc < Job.N0Src[1] + Job.NSrc[1]; jSrc++)
      for (int kSrc = Job.N0Src[2]; kSrc < Job.N0Src[2] + Job.NSrc[2]; kSrc++)
      {

        std::cout << "Starting Source " << iSrc << ", " << jSrc << ", " << kSrc << std::endl;
        // Start of Memory allocation and initialization preamble and problem setup

        // Initialize the source at iSrc, jSrc, kSrc
        InitSource(Source, Job, iSrc, jSrc, kSrc );

        // Allocate and Initialize the Memory for the rays and for the ray-front triangulation
        trirayMem rayArray(Source->GetVWidth()/2);

        // Allocate and Initialize the Memory for the rays and for the ray-front triangulation
        int norays = Source->GetNoRays();
        int ierr = rayArray.reserve(1000000);
#ifdef DEBUG_CHECK
        if ( ierr != 0 )
          std::cerr << "rayArray could not be allocated\n";
#endif
        for (int r = 0; r < norays; r++)
          rayArray.push_back(Source->GetRay(r));

        
        // End of the memory allocation and initialization preamble and problem setup


        for (int ix = 0; ix < NVolx; ix++)
          for (int iy = 0; iy < NVoly; iy++)
            Binning[ix][iy] = false;

#ifdef TIMING
        Timer mytimer;
        double t_sub = 0;
        double t_all = 0;
#endif

        int t_steps=0;

// Main timestep loop of the ray tracer
        while ( t_steps < Job.g_MAXTSTEP)
          {
            t_steps++;
            

            int Tri_Index = 0;

            trirayMem::iterator iter_end = rayArray.end();
            for (trirayMem::iterator iter = rayArray.begin(); iter != iter_end; ++iter)
              {
                triray3D* ray = &(*iter);
                if ( ray->mainray.history != -1)
                  {
                    this->StepTracer(*ray);

                    if (Outside(ray->mainray))
                      {
                        ray->mainray.history = -1;

                        if ( ray->mainray.x[2] > 0)
                          {
                            int ix = (int)((ray->mainray.x[0] - Job.X0Vol[0])/25.0);
                            int iy = (int)((ray->mainray.x[1] - Job.X0Vol[1])/25.0);
                            
                            if ( (ix >= 0) && (ix < NVolx) && (iy >= 0) && (iy < NVoly) )
                              {
                                if (Binning[ix][iy] == false)
                                  {
                                    Binning[ix][iy] = true;
                                    if ( (ray->mainray.x[0] < Job.X0Src[0] + Job.N0Src[0] * Job.dxSrc[0])
                                         || (ray->mainray.x[0] >= Job.X0Src[0] + (Job.N0Src[0] + Job.NSrc[0]) * Job.dxSrc[0])
                                         || (ray->mainray.x[1] < Job.X0Src[1] + Job.N0Src[1] * Job.dxSrc[1])
                                         || (ray->mainray.x[1] >= Job.X0Src[1] + (Job.N0Src[1] + Job.NSrc[1]) * Job.dxSrc[1]) )
                                      {
                                        HitOut++;
                                        if  ( (ray->mainray.x[0] < Source->pos[0] - 5000)
                                         || (ray->mainray.x[0] > Source->pos[0] + 5000)
                                         || (ray->mainray.x[1] < Source->pos[1] - 5000)
                                         || (ray->mainray.x[1] > Source->pos[1] + 5000) )
                                          HitOut5000++;
                                      }
                                    else
                                      HitIn++;
                                    
                                    int it = (int)((t_steps*Job.g_TSTEPSIZE)/TBin);
                                    if ( (it < 0) || (it >= Nt) )
                                      std::cout << "Time Range is not sufficient!\n";
                                    else
                                      TimeHits[it]++;
                                  }
                              }
                          }

                      }
                  }
              }
          }// End of main timestep loop


#ifdef TIMING
        t_all += mytimer.GetTime();
#endif
#ifdef TIMING
        std::cout << "Tracing of " << rayArray.size() << " rays for " << t_steps << " time steps\n";
        std::cout << "This took " << t_all << " seconds\n";
        std::cout << "The subdivision took " << t_sub << " seconds\n";
#endif

//         trirayMem::iterator iter_end = rayArray.end();
//         for (trirayMem::iterator iter = rayArray.begin(); iter != iter_end; ++iter)
//           {
//             ray3D* ray = &(*iter);
//             if ( ray->x[2] > 0)
//               {
//                 int ix = (int)((ray->x[0] - Job.X0Vol[0])/25.0);
//                 int iy = (int)((ray->x[1] - Job.X0Vol[1])/25.0);
                
//                 if ( (ix >= 0) && (ix < NVolx) && (iy >= 0) && (iy < NVoly) )
//                   {
//                     if (Binning[ix][iy] == false)
//                       {
//                         Binning[ix][iy] = true;
//                         if ( (ray->x[0] < Job.X0Src[0] + Job.N0Src[0] * Job.dxSrc[0])
//                              || (ray->x[0] >= Job.X0Src[0] + (Job.N0Src[0] + Job.NSrc[0]) * Job.dxSrc[0])
//                              || (ray->x[1] < Job.X0Src[1] + Job.N0Src[1] * Job.dxSrc[1])
//                              || (ray->x[1] >= Job.X0Src[1] + (Job.N0Src[1] + Job.NSrc[1]) * Job.dxSrc[1]) )
//                           HitOut++;
//                         else
//                           HitIn++;

//                         int it = (int)((t_steps*Job.g_TSTEPSIZE)/TBin);
//                         if ( (it < 0) || (it >= Nt) )
//                           std::cout << "Time Range is not sufficient!\n";
//                         else
//                           TimeHits[it]++;
//                       }
//                   }
//               }
//           }
      }// End of loop over Sources

  std::cout << "Hits inside Surface Area  : " << HitIn << std::endl;
  std::cout << "Hits outside Surface Area : " << HitOut << std::endl;
  std::cout << "Hits outside Source +- 5000 : " << HitOut5000 << std::endl;
  std::cout << "Ratio total                 : " << ((float)(HitIn))/(HitIn+HitOut) << std::endl;
  std::cout << "Ratio in 5000               : " << ((float)(HitIn))/(HitIn+(HitOut-HitOut5000)) << std::endl;

  for (int it = 0; it < Nt; it++)
    std::cout << it << "   " << TimeHits[it] << std::endl;
 
 if (Source != NULL)
      delete Source;
}

template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::shootwf(const TracingJob& Job,
			      const point3D<float> SrcPoint, const point3D<float>& direction, const float& aperture,
			      const grid3D _RcvGrid,
			      float* tt, Spherical* dir, const int ith)
{
    ReceiverGrid* RcvGrid = new ReceiverGrid(_RcvGrid);    
    
    BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);
    ObserverOperator<TracingOperator_T> Observer(&this->StepTracer, ith);
    
    ReflectionOperator Reflect;

    WFPtSrc* Source = NULL;
    VREPR_T velocity;
    this->StepTracer.GetVelocityAt(velocity, SrcPoint, ith);
    Source = new  WFPtSrc(SrcPoint, velocity, deg2rad(Job.g_InitAngle), direction, deg2rad(aperture), 0);

    tracewf(Job, Source, RcvGrid, Observer, Outside, ith);

    if (Source != NULL)
	delete Source;

    int index = 0;
    for (int ix = 0; ix < Job.NRcv[0]; ix++)
	for (int iy = 0; iy < Job.NRcv[1]; iy++)
	    for (int iz = 0; iz < Job.NRcv[2]; iz++)
	    {
		Receiver* rcv = RcvGrid->GetPointAt(ix, iy, iz);
		RecSig* Signal = rcv->GetSig();
		tt[index] = Signal->GetT();
		dir[index] = Signal->GetStartDir();
		index++;
	    }

    delete RcvGrid;
}
 
template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::shootwf(const TracingJob& Job,
			      const point3D<float> SrcPoint, const point3D<float>& direction, const float& aperture,
       			      const int nRcv, Receiver* Rcv,
			      float* tt, Spherical* dir, point3D<float>* dirVector, const int ith)
{
    BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);
    ObserverOperator<TracingOperator_T> Observer(&this->StepTracer, ith);
    
    ReflectionOperator Reflect;

    WFPtSrc* Source = NULL;
    VREPR_T velocity;
    this->StepTracer.GetVelocityAt(velocity, SrcPoint, ith);
    Source = new  WFPtSrc(SrcPoint, velocity, deg2rad(Job.g_InitAngle), direction, deg2rad(aperture), 0);

    tracewf(Job, Source, nRcv, Rcv, Observer, Outside, ith);

    if (Source != NULL)
	delete Source;

    int index = 0;
    for (int i = 0; i < nRcv; i++){
	RecSig* Signal = Rcv[i].GetSig();
	tt[i] = Signal->GetT();
	dir[i] = Signal->GetStartDir();
	dirVector[i] = Signal->GetStartDirVector();
    }
}

template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::shootwf(const TracingJob& Job,
			      const point3D<float> SrcPoint, const point3D<float>& direction, const float& aperture,
       			      const int nRcv, point3D<float> *Rcvs,
			      float* tt, Spherical* dir, point3D<float>* dirVector, const int ith)
{
    BoundaryOperator Outside(Job.X0Vol, Job.X1Vol);
    ObserverOperator<TracingOperator_T> Observer(&this->StepTracer, ith);
    
    ReflectionOperator Reflect;

    Receiver* Rcv = new Receiver[nRcv];
    for(int i=0;i<nRcv;i++) 
      Rcv[i]= Receiver(Rcvs[i]);

    WFPtSrc* Source = NULL;
    VREPR_T velocity;
    this->StepTracer.GetVelocityAt(velocity, SrcPoint, ith);
    Source = new  WFPtSrc(SrcPoint, velocity, deg2rad(Job.g_InitAngle), direction, deg2rad(aperture), 0);

    tracewf(Job, Source, nRcv, Rcv, Observer, Outside, ith);

    if (Source != NULL)
	delete Source;

    int index = 0;
    for (int i = 0; i < nRcv; i++){
	RecSig* Signal = Rcv[i].GetSig();
	tt[i] = Signal->GetT();
	dir[i] = Signal->GetStartDir();
	dirVector[i] = Signal->GetStartDirVector();
    }

    delete [] Rcv;
}

/** No descriptions */
template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::shootray(const point3D<float> SrcPoint, const Spherical& direction, 
			       const float& dt, const int& Nt,
			       point3D<float>* r)
{
  int ierr;

  this->StepTracer.Init(dt);

  WFPtSrc* Source = NULL;
  VREPR_T velocity;
  this->StepTracer.GetVelocityAt(velocity, SrcPoint);
  Source = new  WFPtSrc(SrcPoint, velocity);

  ray3D ray = Source->CreateRay(direction);
        
// Main timestep loop of the ray tracer
  int t_steps=0;
  while ( t_steps < Nt)
  {
      r[t_steps] = ray.x;
      this->StepTracer(ray);
      t_steps++;
  }// End of main timestep loop

  if (Source != NULL)
      delete Source;
}

template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::tracewf(const TracingJob& Job, WFPtSrc* Source,
			      ReceiverGrid* RcvGrid, 
			      ObserverOperator<TracingOperator_T>& Observer,
			      BoundaryOperator& Outside,
			      const int ith)
{
        // Init Refinement Operator
        RefinementOperator<TracingOperator_T>* SubDivide = new RefinementOperator<TracingOperator_T>(Job.g_REF_LEN, &this->StepTracer, Source, ith);
        
        // Re-Initialize the receiver grid
        RcvGrid->clear();
        

        // Allocate and Initialize the Memory for the rays and for the ray-front triangulation
         trirayMem rayArray(Source->GetVWidth()/2);
         triMem TriangleList;
         InitRays( rayArray, TriangleList, Source);
	 
        // End of the memory allocation and initialization preamble and problem setup


#ifdef DEBUG_CHECK
        Checker.CheckTriangleList(TriangleList);
#endif

#ifdef OUTPUT_TRIANG
        Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), 0);
#endif


#ifdef TIMING
        Timer mytimer;
        double t_sub = 0;
        double t_all = 0;
#endif

        int t_steps=0;

// Main timestep loop of the ray tracer
	for (;  t_steps < Job.g_MAXTSTEP; t_steps++)
          {
	      //t_steps++;
            

            int Tri_Index = 0;
            triMem::iterator iter_end = TriangleList.end();
            for (triMem::iterator iter = TriangleList.begin(); iter != iter_end; ++iter)
              {
                Tri_Index++;
                Triangle* Tri = &(*iter);
                if (Tri->lifesign != Triangle::DELETED){
                  Tri->status = ( Tri->status == false);

#ifdef TIMING
                  t_triangle += mytimer.GetTime();
#endif

                  // start vector ray loop
                  triray3D* rays[4];
                  triray3D ray_dummy = *(Tri->points[0]);
                  int index = 0;
                  for (int i = 0; i < 3; i++)
                    {
                      if ( Tri->points[i]->mainray.status != Tri->status )
                        {
                          rays[index] = Tri->points[i];
                          rays[index]->mainray.status = (rays[index]->mainray.status == false);
                          index++;
                        }
                    }
                  //        std::cout << index << std::endl;
                  if (index != 0)
                    {
                      if (index < 4)
                        {
                          if (Tri->neighs[0] != NULL)
                            {
                              for (int i = 0; i < 3; i++)
                                {
                                  if (Tri->neighs[0]->points[i]->mainray.status != Tri->status)
                                    {
                                      rays[index] = Tri->neighs[0]->points[i];
                                      rays[index]->mainray.status = (rays[index]->mainray.status == false);
                                      index++;
                                      break;
                                    }
                                }
                            }
                        }
                      if (index < 4)
                        {
                          if (Tri->neighs[1] != NULL)
                            {
                              for (int i = 0; i < 3; i++)
                                {
                                  if (Tri->neighs[1]->points[i]->mainray.status != Tri->status )
                                    {
                                      rays[index] = Tri->neighs[1]->points[i];
                                      rays[index]->mainray.status = (rays[index]->mainray.status == false);
                                      index++;
                                      break;
                                    }
                                }
                            }
                        }
                      if (index < 4)
                        {
                          if (Tri->neighs[2] != NULL)
                            {
                              for (int i = 0; i < 3; i++)
                                {
                                  if (Tri->neighs[2]->points[i]->mainray.status != Tri->status )
                                    {
                                      rays[index] = Tri->neighs[2]->points[i];
                                      rays[index]->mainray.status = (rays[index]->mainray.status == false);
                                      index++;
                                      break;
                                    }
                                }
                            }
                        }

                      if (index <= 4)
                        for (int n = 0; n < index; n++)
                          {
                            this->StepTracer(*rays[n], ith);
                          }
//                       else
//                         {
// 			    for (int n = index; n < 4; n++)
//                             rays[n] = &ray_dummy;

//                           StepTracer(rays, ith);


//                         }
                    }


                  int nrcv = 0;
                  Receiver* RcvTmp[4];

                  if ( Tri->lifesign != Triangle::DELETED)
                    {
                      ReceiverGrid::iterator iter = RcvGrid->begin(Tri);
                      if (!iter.end())
                        {
                          Observer.Init(Tri);
                          for (; !iter.end(); ++iter)
                            {
                              Receiver* Rcv = (*iter);
                              if ( (Rcv->active)) // && (Rcv->LastTstep < (t_steps-1)) )
                                if ( Observer.RecvInTube(Tri, Rcv))
                                  {
                                    RcvTmp[nrcv] = Rcv;
                                    nrcv++;
                                    if (nrcv == 4)
                                      {
                                        for (int ircv = 0; ircv < nrcv; ircv++)
                                          Observer(Tri, RcvTmp[ircv], t_steps+1);
                                        //Observer(Tri, RcvTmp, t_steps);
                                        nrcv = 0;
                                      }
                                  }
                            }
                          for (int ircv = 0; ircv < nrcv; ircv++)
                            Observer(Tri, RcvTmp[ircv], t_steps+1);
                        }
                    }

                  for (int i = 0; i < 3; i++)
                    {
                      if (Outside(Tri->points[i]->mainray))
                        {
                          SubDivide->DeleteTri(Tri);
                          break;
                        }
                    }


                }
              }
            

#ifdef OUTPUT_TRIANG
            if ((t_steps % 1) == 0)
              Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), t_steps/1);
#endif

#ifdef TIMING
            double t_sub_a = mytimer.GetTime();
#endif

            
            (*SubDivide)(TriangleList, rayArray, t_steps*Job.g_TSTEPSIZE);
            

#ifdef DEBUG_CHECK
        Checker.CheckTriangleList(TriangleList);
#endif

#ifdef TIMING
            double t_sub_e = mytimer.GetTime();
            t_all += t_sub_a + t_sub_e;
            t_sub += t_sub_e;
#endif

#ifdef OUTPUT_TRIANG
            if ((t_steps % 1) == 0)
              Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), 10000+t_steps/1);
#endif

          }// End of main timestep loop

        delete SubDivide;
	SubDivide = NULL;

}

template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::tracewf(const TracingJob& Job, WFPtSrc* Source,
			      const int nRcv, Receiver* Rcv, 
			      ObserverOperator<TracingOperator_T>& Observer,
			      BoundaryOperator& Outside,
			      const int ith)
{
        // Init Refinement Operator
        RefinementOperator<TracingOperator_T>* SubDivide = new RefinementOperator<TracingOperator_T>(Job.g_REF_LEN, &this->StepTracer, Source, ith);
        
        // Re-Initialize receivers
        for (int i = 0; i < nRcv; i++)
           Rcv->clear();

        

        // Allocate and Initialize the Memory for the rays and for the ray-front triangulation
         trirayMem rayArray(Source->GetVWidth()/2);
         triMem TriangleList;
         InitRays( rayArray, TriangleList, Source);
	 
        // End of the memory allocation and initialization preamble and problem setup


#ifdef DEBUG_CHECK
        Checker.CheckTriangleList(TriangleList);
#endif

#ifdef OUTPUT_TRIANG
        Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), 0);
#endif


#ifdef TIMING
        Timer mytimer;
        double t_sub = 0;
        double t_all = 0;
#endif

        int t_steps=0;

// Main timestep loop of the ray tracer
	for (;  t_steps < Job.g_MAXTSTEP; t_steps++)
          {
	      //t_steps++;
            

            int Tri_Index = 0;
            triMem::iterator iter_end = TriangleList.end();
            for (triMem::iterator iter = TriangleList.begin(); iter != iter_end; ++iter)
              {
                Tri_Index++;
                Triangle* Tri = &(*iter);
                if (Tri->lifesign != Triangle::DELETED){
                  Tri->status = ( Tri->status == false);

#ifdef TIMING
                  t_triangle += mytimer.GetTime();
#endif

                  // start vector ray loop
                  triray3D* rays[4];
                  triray3D ray_dummy = *(Tri->points[0]);
                  int index = 0;
                  for (int i = 0; i < 3; i++)
                    {
                      if ( Tri->points[i]->mainray.status != Tri->status )
                        {
                          rays[index] = Tri->points[i];
                          rays[index]->mainray.status = (rays[index]->mainray.status == false);
                          index++;
                        }
                    }
                  //        std::cout << index << std::endl;
                  if (index != 0)
                    {
                      if (index < 4)
                        {
                          if (Tri->neighs[0] != NULL)
                            {
                              for (int i = 0; i < 3; i++)
                                {
                                  if (Tri->neighs[0]->points[i]->mainray.status != Tri->status)
                                    {
                                      rays[index] = Tri->neighs[0]->points[i];
                                      rays[index]->mainray.status = (rays[index]->mainray.status == false);
                                      index++;
                                      break;
                                    }
                                }
                            }
                        }
                      if (index < 4)
                        {
                          if (Tri->neighs[1] != NULL)
                            {
                              for (int i = 0; i < 3; i++)
                                {
                                  if (Tri->neighs[1]->points[i]->mainray.status != Tri->status )
                                    {
                                      rays[index] = Tri->neighs[1]->points[i];
                                      rays[index]->mainray.status = (rays[index]->mainray.status == false);
                                      index++;
                                      break;
                                    }
                                }
                            }
                        }
                      if (index < 4)
                        {
                          if (Tri->neighs[2] != NULL)
                            {
                              for (int i = 0; i < 3; i++)
                                {
                                  if (Tri->neighs[2]->points[i]->mainray.status != Tri->status )
                                    {
                                      rays[index] = Tri->neighs[2]->points[i];
                                      rays[index]->mainray.status = (rays[index]->mainray.status == false);
                                      index++;
                                      break;
                                    }
                                }
                            }
                        }

                      if (index <= 4)
                        for (int n = 0; n < index; n++)
                          {
                            this->StepTracer(*rays[n], ith);
                          }
//                       else
//                         {
// 			    for (int n = index; n < 4; n++)
//                             rays[n] = &ray_dummy;

//                           StepTracer(rays, ith);


//                         }
                    }


                  int ncr_rcv = 0; 
                  Receiver* RcvTmp[4];

                  if ( Tri->lifesign != Triangle::DELETED)
                    {
                          Observer.Init(Tri);
		          for (int i = 0; i < nRcv; i++){
				if ( Observer.RecvInTube(Tri, &Rcv[i])){
				    RcvTmp[ncr_rcv] = &Rcv[i];
                                    ncr_rcv++;
                                    if (ncr_rcv == 4){
                                        for (int ircv = 0; ircv < ncr_rcv; ircv++)
                                          Observer(Tri, RcvTmp[ircv], t_steps+1);
                                        ncr_rcv = 0;
                                    }	
				}
		          }
                          for (int ircv = 0; ircv < ncr_rcv; ircv++)
                              Observer(Tri, RcvTmp[ircv], t_steps+1);
                    }

                  for (int i = 0; i < 3; i++)
                    {
                      if (Outside(Tri->points[i]->mainray))
                        {
                          SubDivide->DeleteTri(Tri);
                          break;
                        }
                    }


                }
              }
            

#ifdef OUTPUT_TRIANG
            if ((t_steps % 1) == 0)
              Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), t_steps/1);
#endif

#ifdef TIMING
            double t_sub_a = mytimer.GetTime();
#endif

            
            (*SubDivide)(TriangleList, rayArray, t_steps*Job.g_TSTEPSIZE);
            

#ifdef DEBUG_CHECK
        Checker.CheckTriangleList(TriangleList);
#endif

#ifdef TIMING
            double t_sub_e = mytimer.GetTime();
            t_all += t_sub_a + t_sub_e;
            t_sub += t_sub_e;
#endif

#ifdef OUTPUT_TRIANG
            if ((t_steps % 1) == 0)
              Output.PrintTriangulation(TriangleList, rayArray, TriangleList.size(), 10000+t_steps/1);
#endif

          }// End of main timestep loop

        delete SubDivide;
	SubDivide = NULL;

}

template<class TracingOperator_T>
void WaveFrontTracer<TracingOperator_T>::InitRays(trirayMem& rayArray, triMem& triList, WFPtSrc* Source)
{
  // Allocate and Initialize the Memory for the rays and for the ray-front triangulation
  int norays = Source->GetNoRays();
  int ierr = rayArray.reserve(1000000);
#ifdef DEBUG_CHECK
  if ( ierr != 0 )
    std::cerr << "rayArray could not be allocated\n";
#endif

  std::vector<int> rayIndex(norays, -1);

  int notris = Source->GetNoTriangles();
  ierr = triList.reserve(1000000);
#ifdef DEBUG_CHECK
  if ( ierr != 0 )
    std::cerr << "TriangleList could not be allocated\n";
#endif
  for (int i = 0; i < notris; i++)
    {
      Triangle newTriangle(i);
      triList.push_back(newTriangle);
    }
  

  for (int i = 0; i < notris; i++)
    {
      polygon newtri = Source->GetTri(i);
      int p0 = newtri.v[0];
      int p1 = newtri.v[1];
      int p2 = newtri.v[2];

      if (rayIndex[p0] == -1)
        {
          const int np = rayArray.size();
          rayArray.push_back(triray3D(Source->GetRay(p0)));
          triList[i].points[0] = &rayArray[np];
          rayIndex[p0] = np;
        }
      else
        triList[i].points[0] = &rayArray[rayIndex[p0]];
      
      if (rayIndex[p1] == -1)
        {
          const int np = rayArray.size();
          rayArray.push_back(Source->GetRay(p1));
          triList[i].points[1] = &rayArray[np];
          rayIndex[p1] = np;
        }
      else
        triList[i].points[1] = &rayArray[rayIndex[p1]];
      
      if (rayIndex[p2] == -1)
        {
          const int np = rayArray.size();
          rayArray.push_back(Source->GetRay(p2));
          triList[i].points[2] = &rayArray[np];
          rayIndex[p2] = np;
        }
      else
        triList[i].points[2] = &rayArray[rayIndex[p2]];
      
      int n0 = newtri.n[0];
      int n1 = newtri.n[1];
      int n2 = newtri.n[2];
      
      if ( n0 != -1)
        triList[i].neighs[0] = &triList[n0];
      if ( n1 != -1)
        triList[i].neighs[1] = &triList[n1];
      if ( n2 != -1)
        triList[i].neighs[2] = &triList[n2];
    }
#ifdef DEBUG_CHECK
  Checker.CheckTriangleList(triList);
#endif
}

template<class TracingOperator_T>
void * WaveFrontTracer<TracingOperator_T>::run_ThreadStartup(void *_parm) {
  thread_parm_t* parm = static_cast<thread_parm_t*>(_parm);

  WaveFrontTracer<TracingOperator_T> *tgtObject = parm->Object;
  TracingJob* Job = parm->Job;
  int tid = parm->tid;
  ProgressBar* progress = parm->progress;

  //printf("Running thread object in a new thread\n");
  tgtObject->run(*Job, progress, parm->rank, tid);
  return NULL;
}
