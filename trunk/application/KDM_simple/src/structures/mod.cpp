/***************************************************************************
                          mod.cpp  -  description
   
    Alias class for model coordinates.

                          -------------------
    begin                : Mon Jun 29 2009
    copyright            : (C) 2009 by Dirk Merten
    email                : merten@itwm.fhg.de
***************************************************************************/

#include "mod.h"

MOD operator +(const MOD& A, const MOD& B)
{
    return MOD(A.v + B.v);
};

MOD operator -(const MOD& A, const MOD& B) 
{
    return MOD(A.v - B.v);
};

MOD operator *(const MOD& A, const MOD& B)
{
    return MOD(A.v * B.v);
};

MOD operator /(const MOD& A, const MOD& B)
{
    return MOD(A.v / B.v);
};

float fabsf(const MOD& T)
{
    return fabsf(T.v);
};

float sqrt(const MOD& T)
{
    return sqrt(T.v);
};
