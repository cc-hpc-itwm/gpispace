/*
 * parameters.hpp
 *
 *  Created on: Mar 16, 2011
 *  Author: dr. tiberiu rotaru
 */

#ifndef PORTFOLIO_PARAMS_HPP_
#define PORTFOLIO_PARAMS_HPP_

#include <map>
#include <sstream>
#include <vector>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/access.hpp>

enum column_t { MATURITY, STRIKE, FIXINGS, PV, STDDEV, DELTA, GAMMA, VEGA };

class common_parameters_t
{
public:
	common_parameters_t()
	{
		m_dS = 0.0;
		m_dr = 0.0;
		m_dd = 0.0;
		m_nn = 0;
		m_dSigma = 0.0;
		m_nFirstFixing = 0;
		m_nAnzahlderDividende = 0;
		m_nLBUs = 2; // by default
	}

	common_parameters_t( 	double dS,
							double dr,
							double dd,
							long int nn,
							double dSigma,
							int nFirstFixing,
							long nAnzahlderDividende,
							int nLBUs = 2)
	{
		m_dS = dS;
		m_dr = dr;
		m_dd = dd;
		m_nn = nn;
		m_dSigma = dSigma;
		m_nFirstFixing = nFirstFixing;
		m_nAnzahlderDividende = nAnzahlderDividende;
		m_nLBUs = nLBUs;
	}

	double SpotPrice() { return m_dS; }
	void   setSpotPrice(const double& d) { m_dS =d; }

	double InterestRate() { return m_dr; }
	void   setInterestRate(const double& r) { m_dr = r; }

	double DividendYield() { return m_dd; }
	void   setDividendYield(const double& d) { m_dd = d; }

	long int Iterations(){ return m_nn; }
	void setIterations(const long int& k) { m_nn = k; }

	double Volatility() { return m_dSigma; }
	void   setVolatility(const double& d) { m_dSigma = d; }

	int  FirstFixing() { return m_nFirstFixing; }
	void setFirstFixing(const int& i) { m_nFirstFixing = i; }

	long NumberDividends() { return m_nAnzahlderDividende; }
	void setNumberDividends(const long& l) { m_nAnzahlderDividende = l; }

	int nLBUs() { return m_nLBUs; }
	void setNLBUs(int k) { m_nLBUs = k; }

	template <class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar & m_dS;
		ar & m_dr;
		ar & m_dd;
		ar & m_nn;
		ar & m_dSigma;
		ar & m_nFirstFixing;
		ar & m_nAnzahlderDividende;
	}

private:
	// Spot Price;
	double m_dS;
	// Interest rate
	double m_dr;
	// Dividend yield
	double m_dd;
	// Number of iterations
	long int m_nn;
	// volatility;
	double m_dSigma;
	// first fixing
	int m_nFirstFixing;
	// Number of dividends
	long m_nAnzahlderDividende;

	int m_nLBUs;
};

// specific input parameters, for each row in the worksheet
class row_parameters_t
{
public:
	row_parameters_t()
	{
	}

	row_parameters_t( 	int rowId,
						double dT,
						double dK,
						double dFixingsProJahr )
	{
		m_rowId = rowId;
		m_dT = dT;
		m_dK = dK;
		m_dFixingsProJahr = dFixingsProJahr;
	}

	double& operator()(const int& i)
	{
          switch(i)
            {
            case MATURITY: return Maturity();
            case STRIKE:   return Strike();
            case FIXINGS:  return Fixings();
            default:
              throw std::runtime_error
                ("STRANGE: switch incomplete in row_parameters_t::operator()");
            }
	}

	int rowId() { return m_rowId; }
        void setRowId(int rowId) { m_rowId = rowId; }

	double& Maturity() { return m_dT;}
	void setMaturity(const double& d) { m_dT = d;}

	double& Strike() { return m_dK; }
	void setzStrike(const double& d) { m_dK = d; }

	double& Fixings() { return m_dFixingsProJahr; }
	void setfFixings(const double& d) { m_dFixingsProJahr = d; }

	template <class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar & m_rowId;
		ar & m_dT;
		ar & m_dK;
		ar & m_dFixingsProJahr;
	}

private:
	int m_rowId;

	// Maturity
	double m_dT;
	// Strike
	double m_dK;
	// Number of Fixings per Year
	double m_dFixingsProJahr;
};

// results (corresponding to one row in the worksheet)
class simulation_result_t
{
public:
	simulation_result_t(  int rowId = 0,
						  double dSum1 = 0.0,
	  	  	  	  	  	  double dSum2 = 0.0,
	  	  	  	  	  	  double dDelta = 0.0,
	  	  	  	  	  	  double dGamma = 0.0,
	  	  	  	  	  	  double dVega = 0.0 )
	{
		m_rowId  = rowId;
		m_dSum1  = dSum1;
		m_dSum2  = dSum2;
		m_dDelta = dDelta;
		m_dGamma = dGamma;
		m_dVega  = dVega;
	}

	// PV, STDDEV, DELTA, GAMMA, VEGA
	double operator()(const int& i)
	{
		switch(i)
		{
			case PV: 	 return Value();
			case STDDEV: return StdDev();
			case DELTA:  return Delta();
			case GAMMA:  return Gamma();
			case VEGA:   return Vega();

		}

		return 0;
	}

	int rowId() { return m_rowId; }
	void setRowId(int rowId) { m_rowId = rowId; }

	double Value() { return m_dSum1;}
    void setValue(const double& d) { m_dSum1 = d; }

    double StdDev() { return m_dSum2;}
    void setStdDev(const double& d){ m_dSum2 = d; }

    double Delta() { return m_dDelta; }
    void setDelta(const double& d) { m_dDelta = d; }

    double Gamma() { return m_dGamma; }
    void setGamma(const double& d) { m_dGamma = d; }

    double Vega() { return m_dVega; }
    void setVega(const double& d) { m_dVega = d; }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
    	ar & m_rowId;
    	ar & m_dSum1;
    	ar & m_dSum2;
    	ar & m_dDelta;
    	ar & m_dGamma;
    	ar & m_dVega;
    }

private:
      int m_rowId;

	  /* Results */
	  double m_dSum1;  /* mc() */ // price value
	  double m_dSum2; // standard deviation
	  double m_dDelta; /* (mc (dS + dD1) - mc(dS)) / dD1 */
	  double m_dGamma; /* (mc (dS + 2*dD1) - 2*c(dS) + mc(s)) / dD1^2 */
	  double m_dVega;  /* (mc (dSigma + dD2) - mc(dSigma)) / dD2 */

public:
	  static double gTotalPV;
	  static double gTotalDelta;
	  static double gTotalGamma;
	  static double gTotalVega;
};

typedef std::vector<row_parameters_t> arr_row_parameters_t;
typedef std::vector<simulation_result_t> arr_simulation_results_t;

struct portfolio_data_t
{
	common_parameters_t  common_params;
	arr_row_parameters_t arr_row_params;

	template <class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar & common_params;
		ar & arr_row_params;
	}

	size_t size() { return arr_row_params.size(); }

	std::string cfg( int row_idx, int i )
	{
		if(i<0 || i>3)
			throw;

		std::stringstream sstr;

		if(i==0 || i==3)
			sstr<<"S = "<<common_params.SpotPrice()<<std::endl;
		else
			if(i==1)
				sstr<<"S = "<<1.001*common_params.SpotPrice()<<std::endl;
			else
				if(i==2)
					sstr<<"S = "<<1.002*common_params.SpotPrice()<<std::endl;

		sstr<<"r = "<<common_params.InterestRate()<<std::endl;
		sstr<<"d = "<<common_params.DividendYield()<<std::endl;
		sstr<<"n = "<<common_params.Iterations()<<std::endl;

		if(i==0 || i==1 || i==2 )
			sstr<<"sigma = "<<common_params.Volatility()<<std::endl;
		else
			if(i==3)
				sstr<<"sigma = "<<1.001*common_params.Volatility()<<std::endl;

		sstr<<"FirstFixing = "<<common_params.FirstFixing()<<std::endl;

		sstr<<"AnzahlderDividende = "<<common_params.NumberDividends()<<std::endl;

		//for( int row_idx = 0; row_idx<arr_row_params.size(); row_idx++ )
		{
			sstr<<"row = "<<row_idx<<std::endl;
			sstr<<"T = "<<arr_row_params[row_idx].Maturity()<<std::endl;
			sstr<<"K ="<<arr_row_params[row_idx].Strike()<<std::endl;
			sstr<<"FixingsProJahr = "<<arr_row_params[row_idx].Fixings()<<std::endl;
		}

		return sstr.str();
	}

	void PrintToString( std::string& strJobData )
	{
			std::stringstream sstr;
			sstr<<"S = "<<common_params.SpotPrice()<<std::endl;
			sstr<<"r = "<<common_params.InterestRate()<<std::endl;
			sstr<<"d = "<<common_params.DividendYield()<<std::endl;
			sstr<<"n = "<<common_params.Iterations()<<std::endl;
			sstr<<"sigma = "<<common_params.Volatility()<<std::endl;
			sstr<<"FirstFixing = "<<common_params.FirstFixing()<<std::endl;
			sstr<<"AnzahlderDividende = "<<common_params.NumberDividends()<<std::endl;

			for(size_t row_idx = 0; row_idx<arr_row_params.size(); row_idx++ )
			{
				sstr<<"row = "<<row_idx<<std::endl;
				sstr<<"T = "<<arr_row_params[row_idx].Maturity()<<std::endl;
				sstr<<"K ="<<arr_row_params[row_idx].Strike()<<std::endl;
				sstr<<"FixingsProJahr = "<<arr_row_params[row_idx].Fixings()<<std::endl;
			}

			strJobData = sstr.str();
	}

	/*std::string operator[]( int row_idx )
	{
		if( row_idx < 0 || row_idx >= arr_row_params.size() )
			throw ;

		std::stringstream sstr;
		sstr<<"S = "<<common_params.SpotPrice()<<std::endl;
		sstr<<"r = "<<common_params.InterestRate()<<std::endl;
		sstr<<"d = "<<common_params.DividendYield()<<std::endl;
		sstr<<"n = "<<common_params.Iterations()<<std::endl;
		sstr<<"sigma = "<<common_params.Volatility()<<std::endl;
		sstr<<"FirstFixing = "<<common_params.FirstFixing()<<std::endl;
		sstr<<"AnzahlderDividende = "<<common_params.NumberDividends()<<std::endl;

		sstr<<"row = "<<row_idx<<std::endl;
		sstr<<"T = "<<arr_row_params[row_idx].Maturity()<<std::endl;
		sstr<<"K ="<<arr_row_params[row_idx].Strike()<<std::endl;
		sstr<<"FixingsProJahr = "<<arr_row_params[row_idx].Fixings()<<std::endl;

		return sstr.str();
	}*/

	std::vector<std::string> operator[]( size_t row_idx )
	{
		if( row_idx >= arr_row_params.size() )
			throw ;

		std::vector<std::string> arrCfgs;
		for(int k=0; k<4;k++)
			arrCfgs.push_back( cfg(row_idx, k) );

		return arrCfgs;
	}

	std::string encode() const
	{
		std::ostringstream sstr;
		boost::archive::text_oarchive ar(sstr);
		ar << *this ;

		return sstr.str();
	}

	void decode(const std::string& strMsg )
	{
		std::stringstream sstr(strMsg);
		boost::archive::text_iarchive ar(sstr);
		ar >> *this;
	}
};

struct portfolio_result_t
{
	arr_simulation_results_t arr_sim_results;
	template <class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar & arr_sim_results;
	}

	std::string encode() const
	{
		 std::ostringstream sstr;
		 boost::archive::text_oarchive ar(sstr);
		 ar << *this ;

		 return sstr.str();
	}

	void decode(const std::string& strMsg )
	{
		std::stringstream sstr(strMsg);
		boost::archive::text_iarchive ar(sstr);
		ar >> *this;
	}
};

#endif /* PORTFOLIO_PARAMS_HPP_ */
