/***************************************************************************
                          taper.cpp  -  description
                             -------------------
    begin                : Thu Oct 18 2007
    copyright            : (C) 2007 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "taper.h"

Taper::Taper(const float _min, const float _max, const float _width):taper_array(NULL),notaper(true)
{
    Init(_min, _max, _width);
}

Taper::~Taper()
{
    if (taper_array != NULL)
	delete[] taper_array;
}

void Taper::Init(const float _min, const float _max, const float _width)
{
    fmin = _min; fmax = _max; width = _width;
    if ( fmax < fmin )
    {
	
	Narray = 0;
    }
    else
	Narray  = 99;

    notaper = false;
    if ( width < 1e-6)
    {
	Narray = 0;
	notaper = true;
    }

    if (taper_array != NULL)
	delete[] taper_array;
    taper_array = new float[Narray+1];

    for (int i = 0; i < Narray; i++)
    {
	float val = (1.0 / (Narray+1)) * (i+1);
	taper_array[i] = val;
    }
    taper_array[Narray] = 1.0f;
}


float Taper::operator()(const float val) const
{
    if (notaper)
	return 1.0f;

    int index = Narray;
    if (( val < fmin) || (val > fmax))
	return 0;
    else
    {
	if ( (val - fmin) < width)
	    index = (int)((val - fmin)/width * 99.4f + 0.5);
	else 
	    if ( (fmax - val) < width)
		index = (int)((fmax - val)/(width/99.4f) + 0.5);
    }
    return taper_array[index];
}
