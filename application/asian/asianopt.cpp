#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <vector>
#include <random>
#include "asianopt.h"
#include <stdexcept>

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


roll_result_type AsianMonteCarlo (
    param_t *pstParam
  , unsigned long number_of_rolls
  , unsigned long seed
 )
{
  using engine_type = std::mt19937_64;
  using distribution_type = std::normal_distribution<double>;

  engine_type random_engine (seed);
  distribution_type random_distribution;

  int LastFixing;


  long AnzahlderFixings;
  long lFixing;
  double dx;


	AnzahlderFixings = (long)(pstParam->m_dFixingsProJahr * pstParam->m_dT);

	LastFixing = AnzahlderFixings;

	double *TimeV = new double[LastFixing + 1];
	double *GewV = new double[LastFixing + 1];

	GewV[0] = 0.0;
	for (lFixing=1; lFixing<=AnzahlderFixings; lFixing++)
		GewV[lFixing] = 1.0;

	dx = pstParam->m_dT / 365.0;

	TimeV[0] = 0.0;

	TimeV[AnzahlderFixings] = pstParam->m_dT;
	for (lFixing=(AnzahlderFixings-1); lFixing>=1; lFixing--)
		TimeV[lFixing] = TimeV[lFixing + 1] - dx;

	for (lFixing=0; lFixing<=AnzahlderFixings; lFixing++)
  {
		//printf("\n%e - %e", TimeV[lFixing], GewV[lFixing]);
  }

    if (!CheckAsianParameters (pstParam, LastFixing,TimeV,GewV))
  {
    throw std::logic_error ("CheckAsianParameters");
  };

  // Berechnung des reduzierten Spots;
  double S0 = pstParam->m_dS;

  //double TimeSteps = LastFixing-1;

  double S1,S2;    // Spot
  double GA1,GA2;  // Geometrische Mittel
  double A1,A2;    // Arithmetisce Mittel
  double StdNorm;  // Variable f�r Zufallszahl

  double mu = (pstParam->m_dr-pstParam->m_dd- pstParam->m_dSigma*pstParam->m_dSigma/2.0); // Drift
  double Sum1=0;          // Summen zur Berechnung der Varianz und des Erwartungswerts
  double Sum2=0;
  double Ak = 0;        // Multiplikatoren f�r Optionstypen
  double Sk = 0;        // Multiplikatoren f�r Optionstypen
  double Kk = 0;        // Multiplikatoren f�r Optionstypen
  double DeltaT, GewT;    // Hilfsvariablen
  double value = 0.0;
  double valuecv = 0.0;   // Wert bei geom. Mittel
  double cv;              // Multiplikator falls mit Controle Variate
  double G;               // Hilfsvariable

  double EfGew = 0.0;
  for (int i=pstParam->m_nFirstFixing; i<=LastFixing; i++) EfGew += GewV[i];


  // Festlegung der Berechnungskoeffizienten
  switch (pstParam->type)
  {
      case FixC : Ak= 1.0;Sk= 0.0;Kk=-1.0;pstParam->m_dK=pstParam->m_dK;break;
      case FixP : Ak=-1.0;Sk= 0.0;Kk= 1.0;pstParam->m_dK=pstParam->m_dK;break;
      case FloP : Ak= 1.0;Sk=-1.0;Kk= 1.0;pstParam->m_dK=0.0;break;
      case FloC : Ak=-1.0;Sk= 1.0;Kk=-1.0;pstParam->m_dK=0.0;break;
      default : printf("Switcherror");
  };

  // Berechnung der anal. L�sung der Controle Variate
  if (pstParam->controle_variate)
  {
    cv = 1.0;
  }
  else
  {
    cv = 0.0;
  };

  for (unsigned long roll (0); roll < number_of_rolls; ++roll)
  {
    DeltaT = TimeV[pstParam->m_nFirstFixing]-TimeV[0];
    GewT   = GewV[pstParam->m_nFirstFixing];
    StdNorm = random_distribution (random_engine);

    // Spot1, Spot2 (AntiThetic)
    S1 = exp(mu*DeltaT-pstParam->m_dSigma*sqrt(DeltaT)*StdNorm);
    S2 = exp(mu*DeltaT+pstParam->m_dSigma*sqrt(DeltaT)*StdNorm);

    // Initialisierung des Aritmetischen Mittels
    A1 = S1*GewT;
    A2 = S2*GewT;

    // Initialisierung des Geomtrischen Mittels
    GA1 = pow(S1,GewV[pstParam->m_nFirstFixing]);
    GA2 = pow(S2,GewV[pstParam->m_nFirstFixing]);

    for (int i=(pstParam->m_nFirstFixing+1); i<=LastFixing; i++)
    {
      DeltaT = TimeV[i]-TimeV[i-1];

      StdNorm = random_distribution (random_engine);

      G = GewV[i];

      S1 = S1*exp(mu*DeltaT-pstParam->m_dSigma*sqrt(DeltaT)*StdNorm);
      S2 = S2*exp(mu*DeltaT+pstParam->m_dSigma*sqrt(DeltaT)*StdNorm);

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

    value = 0.5*(std::max(Sk*S1*S0 + Ak*A1 + Kk*pstParam->m_dK,0.0)+
                 std::max(Sk*S2*S0 + Ak*A2 + Kk*pstParam->m_dK,0.0));
    valuecv = 0.5*(std::max(Sk*S1*S0 + Ak*GA1 + Kk*pstParam->m_dK,0.0)+
                   std::max(Sk*S2*S0 + Ak*GA2 + Kk*pstParam->m_dK,0.0));

    value = value-cv*valuecv;

    Sum1 +=value;
    Sum2 +=value*value;

  };

	return {Sum1, Sum2};
};

bool CheckAsianParameters (
  param_t *pstParam,
  int LastFixing,
  double *TimeV,
  double *GewV)

{
	bool OK = true;
	if ( pstParam->m_dS<=0.0 || pstParam->m_dK<0.0 ||
         pstParam->m_dSigma<=0.0 ||
         pstParam->m_dr<0.0 || pstParam->m_dT <= 0.0) OK = false;
	if (pstParam->m_nFirstFixing >= LastFixing || pstParam->m_nFirstFixing <= 0) OK = false;
	return OK;
};
