#include<vector>
//#include<windows.h>

#ifndef __BASICS_H__
#define __BASICS_H__

#if 1
enum YesNoTyp {Yes,No};
enum OptionTyp {Call,Put};
enum ExerciseTyp {American,European};
enum CalculationTyp {Crude,Imp};
#else
typedef enum YesNoTyp {Yes,No};
typedef enum OptionTyp {Call,Put};
typedef enum ExerciseTyp {American,European};
typedef enum CalculationTyp {Crude,Imp};
#endif

#endif


double max (double value1, double value2);
double min (double value1, double value2);


int imax (int value1, int value2);
int imin (int value1, int value2);



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

double Digital ( double S,
    			 double K,
				 double r,
				 double d,
				 double sigma,
				 double T,
				 double Q);

double BlackStd( double S,
				 double K,
				 double r,
				 double d,
				 double sigma,
				 double T,
				 OptionTyp Art);

double BlackStdDiv ( double S,
					 double K,
					 double r,
					 double d,
					 double sigma,
					 double T,
					 Dividenden &Div,
					 OptionTyp Art);


double PayoffCall (double S, double K);
double PayoffPut (double S, double K);

double RitchkenBinomial ( double S,
						  double K,
						  double r,
						  double d,
						  double sigma,
						  double T,
						  Dividenden &Div,
						  OptionTyp Art,
						  ExerciseTyp Exercise,
						  CalculationTyp CalcTyp,
						  int TimeSteps,
						  double &delta,
						  double &gamma,
						  double &theta);


double BlackScholesWarrant ( double S0,
  						     double K,
							 double r,
							 double d,
							 double sigma,
							 double T,
							 Dividenden &Div,
							 long int NShares,
							 long int NWarrants);

int  ChangeGrid (long int n);

double BlackScholesExplizit( double S,
							 double K,
				             double r,
				             double d,
				             double sigma,
				             double T,
				             OptionTyp Art,
							 int Steps);


