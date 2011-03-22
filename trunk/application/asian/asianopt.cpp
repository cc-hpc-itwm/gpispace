#include <cstdio>
#include <cstdlib>
#include <math.h>
//#include "StdAfx.h"
#include <vector>
#include "basics.h"
#include "random.h"
#include "prob.h"
#include "asianopt.h"

// ****************************************************************************
// *****     Asian - Option                                               *****
// *****                                                                  *****
// *****     Fixed-Strike-Call   : max[A(T)-K,0]                          *****
// *****     Fixed-Strike-Put    : max[K-A(T),0]                          *****
// *****     Floating-Strike-Call: max[S(T)-A(T),0]                       *****
// *****     Floating-Strike-Put : max[A(T)-S(T),0]                       *****
// *****     A(T) arithmetisches Mittel , A(T) = A(t1)+A(t2)+....+A(tn)   *****
// *****                                                                  *****
// *****     MonteCarlo mit geometrischem Mittel als Controle Variate     *****
// *****     Quelle: Eigenentwicklung                                     *****
// ****************************************************************************


double AsianMonteCarlo ( 
  double &Ergebnis,
  double &StdDev,
  param_t *pstParam,
  int LastFixing,
  double *TimeV,
  double *GewV,
  //std::vector<double> &TimeV,
  //std::vector<double> &GewV,
  Dividenden &Div,
  AsianTyp Art,
  bool CVBool
  )
{
  // Initialisierung des Pseudo-Zufallsgenerators

  int ran = 0;
#ifdef __USE_GNU
  FILE *fp = fopen("/dev/urandom", "r");
	
  fread(&ran,4,1,fp);
  fclose(fp);
#else /* __LINUX__ */
  ran = time(NULL);
  
#endif /* __LINUX__ */

  NormalNumber Norm (0,1.0);
  Norm.SetSeed(ran);

//	printf("\npid = %i\n",ran);



    if (!CheckAsianParameters (pstParam, LastFixing,TimeV,GewV,Div))
  {
    return -999.999;
  };
  
	// Berechnungs des arithmetischen Mittels der Dividende als Bond
  double DivMean = Div.Mean(pstParam->m_nFirstFixing,LastFixing,
                            pstParam->m_dr,pstParam->m_dT,TimeV,GewV);
  // Berechnung des reduzierten Spots;
  double S0 = Div.FitS(0.0,
                       pstParam->m_dT,
                       pstParam->m_dr,
                       pstParam->m_dS);	
  
  //double TimeSteps = LastFixing-1; 
	
  double S1,S2;    // Spot
  double GA1,GA2;  // Geometrische Mittel
  double A1,A2;    // Arithmetisce Mittel
  double StdNorm;  // Variable f�r Zufallszahl
	
  double mu = (pstParam->m_dr-pstParam->m_dd- pstParam->m_dSigma*pstParam->m_dSigma/2.0); // Drift
  double Sum1=0;          // Summen zur Berechnung der Varianz und des Erwartungswerts
  double Sum2=0;
  //double Varianz;
  //double Wert;    // R�ckgabewerte
  double Ak = 0;        // Multiplikatoren f�r Optionstypen
  double Sk = 0;        // Multiplikatoren f�r Optionstypen
  double Kk = 0;        // Multiplikatoren f�r Optionstypen
  double DeltaT, GewT;    // Hilfsvariablen
  double value = 0.0;
  double valuecv = 0.0;   // Wert bei geom. Mittel
  double valueana = 0.0;  // Analytischer Wert bei geom. Mittel
  double cv;              // Multiplikator falls mit Controle Variate
  double G;               // Hilfsvariable

  double EfGew = 0.0;
  for (int i=pstParam->m_nFirstFixing; i<=LastFixing; i++) EfGew += GewV[i];

  
  // Festlegung der Berechnungskoeffizienten
  switch (Art)
  {
      case FixC : Ak= 1.0;Sk= 0.0;Kk=-1.0;pstParam->m_dK=pstParam->m_dK-DivMean;break;
      case FixP : Ak=-1.0;Sk= 0.0;Kk= 1.0;pstParam->m_dK=pstParam->m_dK-DivMean;break;
      case FloP : Ak= 1.0;Sk=-1.0;Kk= 1.0;pstParam->m_dK=DivMean;break;
      case FloC : Ak=-1.0;Sk= 1.0;Kk=-1.0;pstParam->m_dK=DivMean;break;
      default : printf("Switcherror");
  };
	
  // Berechnung der anal. L�sung der Controle Variate
  Dividenden leer;
  if (CVBool==true)
  {
    //valueana = GeometricDiskret (S0,K,T,sigma,r,d,pstParam->m_nFirstFixing,LastFixing,TimeV,GewV,leer,Art,false);
    cv = 1.0;
  }
  else
  {
    valueana = 0.0;
    cv = 0.0;
  };
	
  for (long int j=1; j<=pstParam->m_nn; j++)
  {
    DeltaT = TimeV[pstParam->m_nFirstFixing]-TimeV[0];
    GewT   = GewV[pstParam->m_nFirstFixing];
    StdNorm = Norm();
			 
    // Spot1, Spot2 (AntiThetic)
    S1 = exp(mu*DeltaT-pstParam->m_dSigma*sqrt(DeltaT)*StdNorm);
    S2 = exp(mu*DeltaT+pstParam->m_dSigma*sqrt(DeltaT)*StdNorm);
			 
    // Initialisierung des Aritmetischen Mittels
    A1 = S1*GewT;
    A2 = S2*GewT;
			 
    // Initialisierung des Geomtrischen Mittels
    GA1 = pow(S1,GewV[pstParam->m_nFirstFixing]);
    GA2 = pow(S2,GewV[pstParam->m_nFirstFixing]);
			 
    // printf("\nS1: %f   S2:  %f",S1,S2);
    
    for (int i=(pstParam->m_nFirstFixing+1); i<=LastFixing; i++)
    {
      DeltaT = TimeV[i]-TimeV[i-1];
					
      StdNorm = Norm();
					
      G = GewV[i];
      
      S1 = S1*exp(mu*DeltaT-pstParam->m_dSigma*sqrt(DeltaT)*StdNorm);
      S2 = S2*exp(mu*DeltaT+pstParam->m_dSigma*sqrt(DeltaT)*StdNorm);
      //printf("S0=%f S1=%f, S2=%f, G=%f, EfGew=%f\n", S0, S1, S2, G, EfGew);
      
      // Arithmetisches Mittel
      A1 += S1*G;
      A2 += S2*G;

      // Geometrisches Mittel
      GA1 *= pow(S1,G);
      GA2 *= pow(S2,G);
					

    }

    // Korrekte Gewichtung
    A1 = S0 * A1/ EfGew;
    A2 = S0 * A2/ EfGew;

    GA1 = S0 * pow(GA1,1/EfGew);
    GA2 = S0 * pow(GA2,1/EfGew);
    //printf(">>%f + %f + %f = %f\n", 
    //         Sk*S1*S0 ,Ak*A1,  Kk*pstParam->m_dK,
    //         Sk*S1*S0 + Ak*A1 + Kk*pstParam->m_dK);
      
    value = 0.5*(max(Sk*S1*S0 + Ak*A1 + Kk*pstParam->m_dK,0.0)+
                 max(Sk*S2*S0 + Ak*A2 + Kk*pstParam->m_dK,0.0));
    valuecv = 0.5*(max(Sk*S1*S0 + Ak*GA1 + Kk*pstParam->m_dK,0.0)+
                   max(Sk*S2*S0 + Ak*GA2 + Kk*pstParam->m_dK,0.0));

    value = value-cv*valuecv;

    Sum1 +=value;
    Sum2 +=value*value;

  };


  //printf("\nX-Sum1 = %f\n",Sum1);
  //printf("X-Sum2 = %f\n",Sum2);
  // compute final result
  double Wert    = exp(-pstParam->m_dr * pstParam->m_dT) * ( Sum1 / pstParam->m_nn ); 
  //+cv*valueana*exp(r*T) ); --> ignore this part now since valueana is 0 always!
  double Varianz = ( 1.0/pstParam->m_nn * ( Sum2 - 1.0/pstParam->m_nn * Sum1 * Sum1 ) ) / pstParam->m_nn;

  Ergebnis =Wert;
  StdDev   =sqrt(Varianz)*exp(-pstParam->m_dr*pstParam->m_dT);

  //Ergebnis = Sum1;
  //StdDev   = Sum2;

	return 0;
};



#if 0
  
//==============================================================================
//======                   Hilfsfunktion f�r GeometricDiscret              =====
//==============================================================================


void swapdouble (double &a, double &b)
{
	double h = a; a = b; b = h;
}
  
 
// ***************************************************************************
// *****        Spreadberechnung mittels numerischer Integration         *****
// *****        Call: max(X1-X2-K,0)                                     *****
// *****        Put : max(X2-X1+K,0)                                     *****
// *****                                                                 *****
// *****        E(X1) = M1, Var(X1) = Var(X1), usw.                      *****
// *****        Quelle: Zhang                                            *****
// *****                                                                 *****
// *****        ben�tigt f�r Floating Geometric Call/Put                 *****
// ***************************************************************************


double newspread ( double M1,
				   double M2,
				   double Var1,
				   double Var2,
				   double rho,
				   double K,
                   double disc,
				   bool CallPutFlag)

{
	if (K == 0.0) 
	{
		if (CallPutFlag==false)
		{
			swapdouble(M1,M2);
			swapdouble(Var1,Var2);
		};

		double si = sqrt(Var1+Var2-2*rho*sqrt(Var1*Var2));
		double d1 = (M1-M2+0.5*si*si + 0.5*(Var1 - Var2) )/si;
		double d2 = (M1-M2-0.5*si*si + 0.5*(Var1 - Var2) )/si;
		return (disc*(exp(M1+0.5*Var1)*N(d1) - exp(M2+0.5*Var2)*N(d2)));
	}
	else
	{
	
		
		const double step = 0.02;
		
		double A1,A2,A3;
		double phi1,phi2,phi3;
		double v1,v2,v3;
		double d1,d2,d3;
		double NormDens;
		double rho2 = sqrt(1.0-rho*rho);
		double value;

		A1 = A2 = A3 = 0.0;
		
		double d = ( M1 - log(K) ) / sqrt(Var1);
		
		if (K>0) 
		{							
			for (double v=-6.0; v<=6.0-step; v += step)
			{
						
				v1 = v + rho * sqrt(Var1);
				v2 = v + sqrt(Var2);
				v3 = v;						
						
				NormDens = NormalDensity(v);
				
				phi1 = - log( 1.0 + 1.0/K * exp(M2 + v1 * sqrt(Var2)) ) / sqrt(Var1);
				phi2 = - log( 1.0 + 1.0/K * exp(M2 + v2 * sqrt(Var2)) ) / sqrt(Var1);
				phi3 = - log( 1.0 + 1.0/K * exp(M2 + v3 * sqrt(Var2)) ) / sqrt(Var1);

				d1 = ( d + rho*v1  + phi1 ) / rho2 + rho2 * sqrt(Var1);
				d2 = ( d + rho*v2  + phi2 ) / rho2 ;
				d3 = ( d + rho*v   + phi3 ) / rho2;
				
				A1 += NormDens * N(d1);
				A2 += NormDens * N(d2);
				A3 += NormDens * N(d3);
				
			};

			
			value = ( exp(M1+0.5*Var1) * A1 - exp(M2+0.5*Var2) * A2 - K * A3 ) * step * disc;

			
			if (CallPutFlag==false)
			{
				value = value - ( exp(M1+0.5*Var1)  - exp(M2+0.5*Var2) - K) * disc;
			};
		
		}
		else
		{

			value = newspread (M2,M1,Var2,Var1,rho,-K,disc,CallPutFlag);
			if (CallPutFlag==true)
			{
				value += ( exp(M1+0.5*Var1)  - exp(M2+0.5*Var2) - K) * disc;
			}
			else
			{
				value -= ( exp(M1+0.5*Var1)  - exp(M2+0.5*Var2) - K) * disc;
			};
		};

		return value;
	};
};



// ****************************************************************************
// *****     Asian - Option bei geom. Mittel                              *****
// *****                                                                  *****
// *****     Geometrisches Mittel:                                        *****
// *****     Fixed-Strike-Call   : max[G(T)-K,0]                          *****
// *****     Fixed-Strike-Put    : max[K-G(T),0]                          *****
// *****     Floating-Strike-Call: max[S(T)-G(T),0]                       *****
// *****     Floating-Strike-Put : max[G(T)-S(T),0]                       *****
// *****                                                                  *****
// *****     Approximation bei arithmetischem Mittel:                     *****
// *****     Floating-Strike-Call: max[S(T)-G(T)+E(G(T)-A(T)),0]          *****
// *****     Floating-Strike-Put : max[G(T)-E(G(T)-A(T))-S(T),0]          *****
// *****     Fixed-Strike-Call   : max[G(T)-E(G(T)-A(T))-K,0]             *****
// *****     Fixes-Strike-Put                                             *****
// *****                                                                  *****
// *****     A(T) arithmetisches Mittel , A(T) = A(t1)+A(t2)+....+A(tn)   *****
// *****     G(T) geom.Mittel / G(T) =  [A(t1)*A(t2)*....*A(tn)]^[1/n]    *****
// *****                                                                  *****
// *****     Grund-Quelle: Bloomberg: "Modelling Asian Options" (1999)    *****
// *****     by Eric Berger, Oleg Goldshmidt, Dalia Goldwirth-Piran       *****
// ****************************************************************************



double GeometricDiskret (
param_t *pstParam,
  double S,
  double K,
  double T,
  double sigma,
  double r,
  double d,
//int pstParam->m_nFirstFixing,
  int LastFixing,
  std::vector<double> &TimeV,
  std::vector<double> &GewV,
  Dividenden &Div,
  AsianTyp Art,
  bool Arit)
{

  
if (!CheckAsianParameters (pstParam,LastFixing,TimeV,GewV,Div))
  {
	  return -999.999;
  };
  
  
  double DivMean = Div.Mean(pstParam->m_nFirstFixing,LastFixing,r,T,TimeV,GewV);
  double So = Div.FitS(0.0,T,r,S);	
  
  if ( (DivMean != 0) && (Arit==false)) return -999.999;

  
  
  //int TimeSteps = TimeV.size()-1;
  int i,j;
  double EfGew = 0.0;
  double Mm = 0.0;
  double Vm = 0.0;
  double Tm = 0.0;
  double Km = 0.0;
  double EA1 = 0.0;
  double Ms,Vs;
  double Vmrho;
  double value;

  for (i=pstParam->m_nFirstFixing; i<=LastFixing; i++) EfGew += GewV[i];

  for (i=pstParam->m_nFirstFixing; i<=LastFixing; i++)
  {
	  Tm += TimeV[i]*GewV[i]*GewV[LastFixing];
	  Mm += GewV[i] * ( log(So) + (r-d-0.5*sigma*sigma)*TimeV[i] ) ;
		
  };

  Mm /= EfGew;
  Tm /= (EfGew*EfGew);
  
  for (i=pstParam->m_nFirstFixing ; i<=LastFixing; i++)
  {
	  for(j=pstParam->m_nFirstFixing; j<=LastFixing; j++)
	  {
		  Vm += GewV[i]*GewV[j]*min(TimeV[i],TimeV[j]);
	  };
  };

  Vmrho = Vm / (EfGew*EfGew);
  Vm    = Vm * (sigma*sigma) / (EfGew*EfGew);
   
  
  // Berechnung des Strikes
  
  
 if (Arit==true) 
 {
	  for (i=pstParam->m_nFirstFixing; i<=LastFixing; i++)
	  {
		  EA1 += GewV[i]*exp((r-d)*TimeV[i]);
	  };

	  EA1 = EA1*So/EfGew;

	  Km = EA1 - exp(Mm+0.5*Vm) + DivMean;
 }
else
 {
	 Km = 0.0;
 };

  
if (Art==FixC || Art==FixP) 
{
	Km = K-Km;
}

printf("Km:%f  K:%f",Km,K); 
  
  bool CallPutFlag;
  
  if ( Art==FixC || Art==FloC)
  {
	  CallPutFlag=true;
  } 
  else
  {
	  CallPutFlag=false;
  };
  
  if (Art==FixC || Art==FixP)
  {
	return TWBlackScholes(CallPutFlag,Mm,Vm,Km,exp(-r*T));
  }
  else
  {	  
	  double rho = Tm / ( sqrt(T*Vmrho)*GewV[LastFixing]/EfGew);
	  Ms = log(So) + (r-d-0.5*sigma*sigma) * T;
	  Vs = sigma*sigma*T;
	  return  newspread(Ms,Mm,Vs,Vm,rho,Km,exp(-r*T),CallPutFlag);
  };			      

};






double TWBlackScholes ( bool CallPutFlag,
					    double Mean,
						double Var,
						double Strike,
						double disc)
{
	double Th;
	double d2,d1;

	if (CallPutFlag==true) {Th = 1.0;} else {Th = -1.0;};
	
	if (Strike>0) 
	{
		d2 = (Mean - log(Strike)) / sqrt(Var);
		d1 = d2 + sqrt(Var);
		return (disc*Th*( exp(Mean+0.5*Var)*N(Th*d1) - Strike*N(Th*d2) ));
	}
	else
	{
		return disc*Th*( exp(Mean+0.5*Var) - Strike);
	};

};




#endif /* 0 */

bool CheckAsianParameters ( 
  param_t *pstParam,
  int LastFixing,
  double *TimeV,
  double *GewV,
  //std::vector<double> &TimeV,
  //std::vector<double> &GewV,
  Dividenden &Div)

{
	bool OK = true;
	if ( pstParam->m_dS<=0.0 || pstParam->m_dK<0.0 || 
         pstParam->m_dSigma<=0.0 ||
         pstParam->m_dr<0.0 || pstParam->m_dT <= 0.0) OK = false;
	//if (TimeV.size() != GewV.size() || TimeV.size() <2 ) OK = false;
	if (pstParam->m_nFirstFixing >= LastFixing || pstParam->m_nFirstFixing <= 0) OK = false;
	//if (LastFixing > TimeV.size() || Div.FitS(0.0,T,r,S) <= 0) OK = false;
    if (Div.Mean(pstParam->m_nFirstFixing,LastFixing, pstParam->m_dr,
                 pstParam->m_dT, TimeV, GewV) < 0.0) OK = false;	
	return OK;
};



