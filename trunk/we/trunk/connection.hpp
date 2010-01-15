// mirko.rahn@itwm.fraunhofer.de

#ifndef _CONNECTION_HPP
#define _CONNECTION_HPP

template<typename TYPE, typename TID, typename PID>
struct connection
{
public:
  TYPE type;
  TID tid;
  PID pid;

  connection () : type(), tid(), pid() {}

  connection (const TYPE & _type, const TID & _tid, const PID & _pid) 
    : type (_type)
    , tid (_tid)
    , pid (_pid)
  {}
};

#endif
