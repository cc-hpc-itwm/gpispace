/***************************************************************************
                          receiver.cpp  -  description
                             -------------------
    begin                : Wed Jan 25 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#include "receiver.h"

/** Save the signal locally or remote */
bool Receiver::SaveSig(const RecSig& ThisSig, const int& Tstep){
  bool Ret = true;

  if ( Tstep == LastTstep )
  {
    ++pSignal;
    if ((pSignal)->GetT() > ThisSig.GetT())
    {
      (*(pSignal)) = ThisSig;
    }
    --pSignal;
  }
  else
    if (pSignal != Signals)
    {
      (*pSignal) = ThisSig;
      pSignal--;
    }

  LastTstep = Tstep;

  return Ret;  
}
/** Stores the local signal buffer in a remote memory space and frees the local buffer */
bool Receiver::StoreSigBuffer(){
  std::cerr << "WARNING in Receiver:StoreSigBuffer() at " << pos << ": Overflow of local buffer detected\n";
  std::cerr << "                                                              but remote store not implemented yet\n";
  std::cerr << "                                                              Freeing buffer.\n";

  pSignal = &(Signals[g_MAXSIG-1]);

  return false;
}
/** Clear the signal buffer */
void Receiver::clear(){
  pSignal = &(Signals[g_MAXSIG-1]);
  LastTstep = -1;
}
/** Return a pointer to the last recorded Signal */
RecSig* Receiver::GetSig(){
  return &Signals[g_MAXSIG-1];
}

RecSig* Receiver::Get_firstarrival() {
  RecSig* pSig = pSignal;
  if ( pSig == &Signals[g_MAXSIG-1]) // no signal recorded
	return NULL;
  else {
	pSig = &Signals[g_MAXSIG-1];
// 	if ( (pSig->GetT() != pSig->GetT()) || (pSig->GetT()+1 == pSig->GetT()) ) {
	if ( isnan(pSig->GetT()) || (isinf(pSig->GetT())) ) return NULL;
	else return pSig;
  }
}

float Receiver::get_ttime() {
  RecSig* pSig = Get_firstarrival();
  if (pSig==NULL) return INVALID_SIGT; else return pSig->GetT();
}

float Receiver::get_ttime(float& init_x, float& init_y) {
  RecSig* pSig = Get_firstarrival();
  if (pSig==NULL) {
    init_x = INVALID_SIGT; init_y = INVALID_SIGT; return INVALID_SIGT; 
  }
  else {
    Spherical StartDir = pSig->GetStartDir();
    init_x = sin(StartDir.theta)*cos(StartDir.phi);
    init_y = sin(StartDir.theta)*sin(StartDir.phi);
    return pSig->GetT(); 
  }
}



/** operator = (const Receiver& ) */
Receiver& Receiver::operator = (const Receiver& _Recv){
  pos = _Recv.pos;
  active = _Recv.active;
  pSignal = &Signals[g_MAXSIG-1];
  const RecSig* pTmp = &_Recv.Signals[g_MAXSIG-1];
  while (pTmp != _Recv.pSignal)
  {
    *pSignal = *pTmp;
    pTmp--;
    pSignal--;
  }

  return *this;
}
/** Receiver (const Receiver& ) */
Receiver::Receiver (const Receiver& _Recv){
  pos = _Recv.pos;
  active = _Recv.active;
  pSignal = &Signals[g_MAXSIG-1];
  const RecSig* pTmp = &_Recv.Signals[g_MAXSIG-1];
  while (pTmp != _Recv.pSignal)
  {
    *pSignal = *pTmp;
    pTmp--;
    pSignal--;
  }
}
