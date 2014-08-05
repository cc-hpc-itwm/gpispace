#ifndef __Dividenden_C__
#define __Dividenden_C__

class Dividenden

{
// ***************************************************************
//         Struct bzw. Class mit & an Funktionen �bergeben
//         double dfd (Dividenden &Div,double t,double r)
// ***************************************************************
private:



public:

	double *Dw;
	double *t;
	int Nr;

	Dividenden(double* CDw, double* Ct, int CNr);
	Dividenden(double D1, double t1);
	Dividenden(double D1, double t1, double D2, double t2);
	Dividenden(double D1, double t1, double D2, double t2, double D3, double t3);
	Dividenden();

    ~Dividenden();

	double FutureDividendValue (double t0, double  T, double  r);
	double FitS(double t0,double T, double r, double S);
	double Maximum (double t0, double t1, double T, double r);
	double Mean( int FF,
							 int LF,
							 double r,
							 double T,
							 double *TimeV,
						 	 double *GewV);

							 //std::vector<double> &TimeV,
					         //std::vector<double> &GewV);
	double D (int n);
	double T (int n);
	int NextDivNumber(double T);
	void Ausgabe(void);
	void Sort(void);


};

#endif
