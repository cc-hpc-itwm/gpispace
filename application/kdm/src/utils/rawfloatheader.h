/***************************************************************************
                          RawFloatHeader.h  -  description
                             -------------------
    begin                : Thu Feb 23 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


#ifndef RAWFLOATHEADER_H
#define RAWFLOATHEADER_H


/**
  *@author Dirk Merten
  */
struct RawFloatHeader{
  unsigned char segyFile[512];
  unsigned char prName[512];
  int fileInfo[16];
  int userParam[11];
  float max;
  float min;
  int rank;
  int size;
};
#endif //SEGYHEADER_H
  
