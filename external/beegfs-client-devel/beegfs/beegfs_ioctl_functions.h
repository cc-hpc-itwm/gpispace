#ifndef __BEEGFS_IOCTL_FUNCTIONS_H__
#define __BEEGFS_IOCTL_FUNCTIONS_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


static inline bool beegfs_getConfigFile(int fd, char** outCfgFile);
static inline bool beegfs_getRuntimeConfigFile(int fd, char** outCfgFile);
static inline bool beegfs_testIsBeeGFS(int fd);
static inline bool beegfs_getMountID(int fd, char** outMountID);
static inline bool beegfs_getStripeInfo(int fd, unsigned* outPatternType, unsigned* outChunkSize, 
   uint16_t* outNumTargets);
static inline bool beegfs_getStripeTarget(int fd, uint16_t targetIndex, uint16_t* outTargetNumID, 
   uint16_t* outNodeNumID, char** outNodeStrID);
static inline bool beegfs_createFile(int fd, const char* filename, mode_t mode,
   unsigned numtargets, unsigned chunksize);


/**
 * Get the path to the client config file of an active BeeGFS mountpoint.
 * 
 * @param fd filedescriptor pointing to file or dir inside BeeGFS mountpoint.
 * @param outCfgFile buffer for config file path; will be malloc'ed and needs to be free'd by
 *        caller if success was returned.
 * @return true on success, false on error (in which case errno will be set).
 */
bool beegfs_getConfigFile(int fd, char** outCfgFile)
{
   struct BeegfsIoctl_GetCfgFile_Arg getCfgFile;
   getCfgFile.length = BEEGFS_IOCTL_CFG_MAX_PATH;

   int res = ioctl(fd, BEEGFS_IOC_GET_CFG_FILE, &getCfgFile);
   if(res)
      return false;

   *outCfgFile = strndup(getCfgFile.path, BEEGFS_IOCTL_CFG_MAX_PATH);
   if(!*outCfgFile)
      return false;

   return true;
}

/**
 * Get the path to the client runtime config file in procfs.
 * 
 * @param fd filedescriptor pointing to file or dir inside BeeGFS mountpoint.
 * @param outCfgFile buffer for config file path; will be malloc'ed and needs to be free'd by
 *        caller if success was returned.
 * @return true on success, false on error (in which case errno will be set).
 */
bool beegfs_getRuntimeConfigFile(int fd, char** outCfgFile)
{
   struct BeegfsIoctl_GetCfgFile_Arg getCfgFile;
   getCfgFile.length = BEEGFS_IOCTL_CFG_MAX_PATH;

   int res = ioctl(fd, BEEGFS_IOC_GET_RUNTIME_CFG_FILE, &getCfgFile);
   if(res)
      return false;

   *outCfgFile = strndup(getCfgFile.path, BEEGFS_IOCTL_CFG_MAX_PATH);
   if(!*outCfgFile)
      return false;

   return true;
}

/**
 * Test if the underlying file system is a BeeGFS.
 * 
 * @param fd filedescriptor pointing to some file or dir that should be checked for whether it is
 *        located inside a BeeGFS mount.
 * @return true on success, false on error (in which case errno will be set).
 */
bool beegfs_testIsBeeGFS(int fd)
{
   char testArray[sizeof(BEEGFS_IOCTL_TEST_STRING)];

#ifdef BEEGFS_DEBUG
   // just calm valgrind; it does not detect that the array is initialized by the ioctl
   memset(testArray, 0, sizeof(BEEGFS_IOCTL_TEST_STRING) );
#endif

   int ioctlRes = ioctl(fd, BEEGFS_IOC_TEST_IS_BEEGFS, testArray);
   if(ioctlRes)
      return false;

   int memCmpRes = memcmp(testArray, BEEGFS_IOCTL_TEST_STRING, sizeof(BEEGFS_IOCTL_TEST_STRING) );
   if(memCmpRes)
   { // ioctl was accepted by underlying fs, but buffer wasn't filled correctly
      errno = EPROTO;
      return false; // verification through buffer failed, probably just not a beegfs
   }

   return true;
}

/**
 * Get the mountID aka clientID aka nodeID of client mount aka sessionID.
 * 
 * @param fd filedescriptor pointing to some file or dir that should be checked for whether it is
 *        located inside a BeeGFS mount.
 * @return true on success, false on error (in which case errno will be set).
 */
bool beegfs_getMountID(int fd, char** outMountID)
{
   char mountIDBuf[BEEGFS_IOCTL_MOUNTID_BUFLEN];

#ifdef BEEGFS_DEBUG
   // just calm valgrind; it does not detect that the array is initialized by the ioctl
   memset(mountIDBuf, 0, sizeof(mountIDBuf) );
#endif

   int ioctlRes = ioctl(fd, BEEGFS_IOC_GET_MOUNTID, mountIDBuf);
   if(ioctlRes)
      return false;

   *outMountID = strndup(mountIDBuf, sizeof(mountIDBuf) );
   if(!*outMountID)
      return false;
   
   return true;
}

/**
 * Get the stripe info of a file.
 * 
 * @param fd filedescriptor pointing to some file inside a BeeGFS mount.
 * @param outPatternType type of stripe pattern (BEEGFS_STRIPEPATTERN_...)
 * @param outChunkSize chunk size for striping.
 * @param outNumTargets number of targets for striping.
 * @return true on success, false on error (in which case errno will be set).
 */
bool beegfs_getStripeInfo(int fd, unsigned* outPatternType, unsigned* outChunkSize, 
   uint16_t* outNumTargets)
{
   struct BeegfsIoctl_GetStripeInfo_Arg getStripeInfo;
   
   int res = ioctl(fd, BEEGFS_IOC_GET_STRIPEINFO, &getStripeInfo);
   if(res)
      return false;
   
   *outPatternType = getStripeInfo.outPatternType;
   *outChunkSize = getStripeInfo.outChunkSize;
   *outNumTargets = getStripeInfo.outNumTargets;

   return true;
}

/**
 * Get the stripe target of a file (with 0-based index).
 * 
 * @param fd filedescriptor pointing to some file inside a BeeGFS mount.
 * @param targetIndex index of target that should be retrieved (start with 0 and then call this
 *        again with index up to "*outNumTargets-1" to retrieve remaining targets).
 * @param outTargetNumID numeric ID of target at given index.
 * @param outNodeNumID numeric ID to node to which this target is assigned.
 * @param outNodeStrID string ID of the node to which this target is assigned; buffer will be 
 *        alloc'ed and needs to be free'd by caller if success is returned.
 * @return true on success, false on error (in which case errno will be set).
 */
bool beegfs_getStripeTarget(int fd, uint16_t targetIndex, uint16_t* outTargetNumID, 
   uint16_t* outNodeNumID, char** outNodeStrID)
{
   struct BeegfsIoctl_GetStripeTarget_Arg getStripeTarget;
   
   getStripeTarget.targetIndex = targetIndex;

   int res = ioctl(fd, BEEGFS_IOC_GET_STRIPETARGET, &getStripeTarget);
   if(res)
      return false;
   
   *outTargetNumID = getStripeTarget.outTargetNumID;
   *outNodeNumID = getStripeTarget.outNodeNumID;

   *outNodeStrID = strndup(getStripeTarget.outNodeStrID, BEEGFS_IOCTL_CFG_MAX_PATH);
   if(!*outNodeStrID)
      return false;

   return true;
}

/**
 * Create a new regular file with stripe hints.
 *
 * As the stripe pattern cannot be changed when a file is already created, this is an exclusive
 * create, so it will return an error if the file already existed.
 *
 * @param fd filedescriptor pointing to parent directory for the new file.
 * @param filename name of created file.
 * @param mode permission bits of new file (i.e. symbolic constants like S_IRWXU or 0644).
 * @param numtargets desired number of storage targets for striping; 0 for directory default; ~0 to
 *        use all available targets.
 * @param chunksize chunksize per storage target for striping in bytes; 0 for directory default;
 *        must be 2^n >= 64KiB.
 * @return true on success, false on error (in which case errno will be set).
 */
bool beegfs_createFile(int fd, const char* filename, mode_t mode, unsigned numtargets,
   unsigned chunksize)
{
   struct BeegfsIoctl_MkFileWithStripeHints_Arg createFileArg;

   createFileArg.filename = filename;
   createFileArg.mode = mode;

   createFileArg.numtargets = numtargets;
   createFileArg.chunksize = chunksize;

   int res = ioctl(fd, BEEGFS_IOC_MKFILE_STRIPEHINTS, &createFileArg);
   if(res)
      return false;

   return true;
}


#endif /* __BEEGFS_IOCTL_FUNCTIONS_H__ */
