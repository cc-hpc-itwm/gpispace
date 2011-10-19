#ifndef SYSTEMINFORMATION_H
#define SYSTEMINFORMATION_H

#include <string>

/// Shortcut for SystemInformation::getInstance()
#define PSPRO_SYSINFO SystemInformation::getInstance()

/** SystemInformation is a class which provides information about the current system (computer).
 * Right now it only provides username and home directory, but it could be extended so that it also
 * provides information about available memory, memory hierarchies etc.
 *
 * All the information about the system should be collected in the initialize() method, so that later on all
 * getSomething() functions can just return the information which is already there.
 */
class SystemInformation
{
  public:
    /** Returns a pointer to the one existing SystemInformation instance. When this is called the first time, all the
     * information about the system is collected. */
    static const SystemInformation* getInstance();

    /** Returns the username under which the current process runs. Independent from environment variables. */
    inline const char* getUsername() const  { return m_username.c_str(); }

    /** Returns the home directory of the current user. If none is found, /tmp is returned. */
    inline const char* getHomeDir() const   { return m_homedir.c_str(); }

    /** Returns the numeric user id of the current user. */
    inline int getUid() const               { return m_uid;}

    /// Returns the amount of physical RAM on the machine in bytes.
    unsigned long getTotalRAM() const      { return m_ulTotalRAM; }

    /** Returns the amount of free/available RAM on the machine in bytes. Due to virtual memory don't trust it too much.
     * This function is slow, it parses /proc/meminfo. */
    unsigned long getFreeRAM() const;

    /** Returns the amount of memory in bytes currently used for file caching by the OS.
     * This function is slow, it parses /proc/meminfo .*/
    unsigned long getCachedRAM() const;

    /// Returns the number of NUMA nodes this system consists of, i.e. the number of separate main memorys
    int getNbNumaNodes() const { return m_nNumaNodes; }

    /// Returns the number of CPUs this system consists of, each CPU can have multiple cores
    int getNbCpus() const { return m_nCpus; }

    /// Returns the number of cores this system consists of, each core may run multiple hyperthreads
    int getNbCores() const { return m_nCores; }

    /** Returns the number of hyperthreads this system consists of. If there is no hyperthreading, this is equal to
     *  the number of cores. */
    int getNbHyperthreads() const { return m_nHyperthreads; }

    /// Returns how many cores each CPU has.
    int getNbCoresPerCpu() const { return m_nCoresPerCpu; }

    /// Returns how many hyperthreads are running on each core. Without hyperthreading, it returns 1.
    int getNbHyperthreadsPerCore() const { return m_nHyperthreadsPerCore; }

    /// Returns true if this is a NUMA machine (i.e. there are more than 1 NUMA nodes).
    bool isNuma() const { return (m_nNumaNodes > 1); }

    /// Returns true if this machine has hyperthreading enabled (i.e.there are more than 1 hyperthreads per core)
    bool isHyperthreading() const { return (m_nHyperthreadsPerCore > 1); }

  private:
    SystemInformation();
    SystemInformation(const SystemInformation&);
    void initialize();
    void findUserInformation();
    void findMemoryInformation();
    void findCPUTopologyInformation();

    static unsigned long getInformationFromMeminfo(const char* _key);

    static SystemInformation* s_pSystemInformationInstance;

    // user related information
    std::string m_username;
    std::string m_homedir;
    int m_uid;

    // memory related information
    unsigned long m_ulTotalRAM;

    // CPU topology related information
    int m_nNumaNodes;
    int m_nCpus;
    int m_nCores;
    int m_nHyperthreads;

    int m_nCoresPerCpu;
    int m_nHyperthreadsPerCore;
};

#endif
