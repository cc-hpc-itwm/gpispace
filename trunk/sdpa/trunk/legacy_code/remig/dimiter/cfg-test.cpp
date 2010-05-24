#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free
#include <cstring>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>


#include <stdio.h>

#include "cfg.h"

#include "reApplInit.h"
#include "reReadInp.h"
#include "reReadVel.h"
#include "calcOneLevl.h"
#include "reUpdt.h"

using namespace sdpa::modules;

static void c_get_config(cfg_t *my_config, fvmAllocHandle_t global_cfg)
{
  fvmAllocHandle_t scratch = fvmLocalAlloc(sizeof(cfg_t));
  LOG(DEBUG, "getting my config (node " << fvmGetRank() << ")");
  fvmCommHandle_t comm_hdl = fvmGetGlobalData(global_cfg, fvmGetRank()*sizeof(cfg_t), sizeof(cfg_t), 0, scratch);
  LOG(DEBUG, "waiting for finish...");
  fvmCommHandleState_t state = waitComm(comm_hdl);
  LOG(DEBUG, "comm state = " << state);
  fvmLocalFree(scratch);

  // copy from shared mem
  memcpy(my_config, fvmGetShmemPtr(), sizeof(cfg_t));
}

static void c_put_config(cfg_t *my_config, fvmAllocHandle_t global_cfg)
{
  // copy to shared mem
  memcpy(fvmGetShmemPtr(), my_config, sizeof(cfg_t));

  fvmAllocHandle_t scratch = fvmLocalAlloc(sizeof(cfg_t));
  LOG(DEBUG, "putting my config (node " << fvmGetRank() << ")");
  fvmCommHandle_t comm_hdl = fvmPutGlobalData(global_cfg, fvmGetRank()*sizeof(cfg_t), sizeof(cfg_t), 0, scratch);
  LOG(DEBUG, "waiting for finish...");
  fvmCommHandleState_t state = waitComm(comm_hdl);
  LOG(DEBUG, "comm state = " << state);
  fvmLocalFree(scratch);
}

static void c_distribute_configs(const cfg_t *configs, fvmAllocHandle_t global_cfg)
{
  fvmAllocHandle_t scratch = fvmLocalAlloc(sizeof(cfg_t));
  for (std::size_t i=0; i < fvmGetNodeCount(); ++i)
  {
     // copy to shared mem
     LOG(DEBUG, "copying " << sizeof(cfg_t) << " bytes config " << &configs[i] << " of node " << i << " to " << fvmGetShmemPtr());
     memcpy(fvmGetShmemPtr(), &configs[i], sizeof(cfg_t));

     // distribute the config to the node
     LOG(DEBUG, "distributing config to node: " << i);
     fvmCommHandle_t comm_hdl = fvmPutGlobalData(global_cfg, i*sizeof(cfg_t), sizeof(cfg_t), 0, scratch);

     LOG(DEBUG, "waiting for finish...");
     fvmCommHandleState_t state = waitComm(comm_hdl);
     LOG(DEBUG, "comm state = " << state);
  }
  fvmLocalFree(scratch);
}



//---- read_config ----
void read_config(data_t &param) throw (std::exception)
{
  const std::string &config_file = param.at("config_file").token().data();
  
  int num_nodes = fvmGetNodeCount();
  std::cout << "running on " << num_nodes << " nodes" << std::endl;

  cfg_t *configs = new cfg_t[num_nodes];
  //configs[0].i = 0;

  // call the C function
  c_read_config(configs);
  //std::cout << "reading config finished: " << configs[0].i << std::endl;

  // write configs to global memory
  fvmAllocHandle_t global_cfg = fvmGlobalAlloc(sizeof(cfg_t));
  c_distribute_configs(configs, global_cfg);


  //------ dms, add here my stuff -------

  int rank = fvmGetRank();

  //-------- print something ---
  FILE *fp; 
  char fn[100];

     sprintf(fn, "/scratch/dimiter/sdpa/fTrace%d.txt", rank);

     if((fp=fopen(fn, "wt")) != NULL) {
      fprintf(fp, "\n [%d] numNodes:%d\n", rank, num_nodes);
      fprintf(fp, "\n size glb VM-space=5 * %d, handle:%ld", 
              (int) configs[rank].nodalSharedSpaceSize, configs[rank].hndGlbVMspace);
      fprintf(fp, "\n size lcl scratch space= %d, handle:%ld\n", 
               (int) configs[rank].nodalScratchSize, configs[rank].hndScratch);

      fprintf(fp, "\n alloc handle for my glb_config distribution on each node:%ld\n", global_cfg);

      if(configs[rank].memOverflow == 1) fprintf(fp, "\n NB!!!! mem overflow, stop the execution,re-set the sizes !!!\n");
      if(configs[rank].memOverflow == 0) fprintf(fp, "\n glb-VM mem allocation OK, continue\n");
      
      fprintf(fp, "\n initzial VM-glb configuiration over\n");

      fclose(fp);
     }
    

    //------ dms, add here my stuff -------
                          // this is to be executed only on node#0 /master

     reApplInit(&(configs[rank]));

     readAndDistributeInput(&(configs[rank]));

     readVelocity(&(configs[rank]));

     int iw=0;
     int iz=0;
     MKL_Complex8 *pSlc=NULL;
     float *pOutpR = NULL;
     float *pRslt = NULL;

     reCalcOneLevl(&(configs[rank]), iw, iz, pSlc, pOutpR);

     reUpdate(&(configs[rank]), pOutpR, pRslt);

     //rePrefixSum(&(configs[rank]));
     //------ end dms --------



  delete [] configs;

  // store output token
  param["global_cfg"].token().data(global_cfg);
}

SDPA_MOD_INIT_START(cfg-test)
{
  SDPA_REGISTER_FUN(read_config);

  // run a test
  data_t fake_param;
  fake_param["config_file"].token().data("/tmp/foo.cfg");
  read_config(fake_param);
}
SDPA_MOD_INIT_END(cfg-test)
