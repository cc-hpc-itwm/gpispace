#include<vector>
#include"basics.h"
#include <asian.h>

#ifndef __ASIAN_H__
#define __ASIAN_H__

enum AsianTyp {FloC,FloP,FixC,FixP};

#endif



double AsianMonteCarlo ( double &Ergebnis,
			 		     double &StdDev,
                         param_t *pstParam,
						 int LastFixing,
						 double *TimeV,
						 double *GewV,
						 //std::vector<double> &TimeV,
						 //std::vector<double> &GewV,
						 Dividenden &Div,
						 AsianTyp Art,
						 bool CVBool );


/*

double WilkinsonDiskret ( double S,
				          double K,
						  double T,
				          double sigma,
				          double r,
						  double d,				        
						  int FirstFixing,
						  int LastFixing,
						  std::vector<double> &TimeV,
						  std::vector<double> &GewV,
						  Dividenden &Div,
			   	          AsianTyp Typ );


double TWAverageRate ( double S,
					   double K,
					   double si,
					   double r,
					   double d,
					   double T,
					   int FirstFixing,
					   int LastFixing,
					   std::vector<double> &TimeV,
					   std::vector<double> &GewV,
					   Dividenden &Div,
					   AsianTyp Art);

double checkderivatives(double delta);

double GeometricDiskret ( double S,
				          double K,
						  double T,
				          double sigma,
				          double r,
						  double d,
						  int FirstFixing,
						  int LastFixing,
						  std::vector<double> &TimeV,
						  std::vector<double> &GewV,
						  Dividenden &Div,
			   	          AsianTyp Typ, 
						  bool Arit);

double TWBlackScholes ( bool CallPutFlag,
					    double Mean,
						double Var,
						double Strike,
						double disc);


double newspread ( double M1,
				   double M2,
				   double Var1,
				   double Var2,
				   double rho,
				   double K,
                   double disc,
				   bool CallPutFlag);
*/
bool CheckAsianParameters ( param_t *pstParam,
					        int LastFixing,
					        double *TimeV,
						 	double *GewV,
					        //std::vector<double> &TimeV,
					        //std::vector<double> &GewV,
					        Dividenden &Div);
