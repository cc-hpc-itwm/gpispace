#include "monitorwindow.hpp"
#include "portfolioeval.hpp"
#include "ui_monitorwindow.h"
#include <QDebug>

#include <sdpa/client/ClientApi.hpp>

double simulation_result_t::gTotalPV 	= 0.0;
double simulation_result_t::gTotalDelta = 0.0;
double simulation_result_t::gTotalGamma = 0.0;
double simulation_result_t::gTotalVega 	= 0.0;

#include <vector>
#include <sstream>
#include <QtGlobal>
#include <QTime>

#include <we/we.hpp>
//#include <sdpa/engine/IWorkflowEngine.hpp>

int Portfolio::RandInt(int low, int high)
{
	// Random number between low and high
	return qrand() % ((high + 1) - low) + low;
}

Portfolio::Portfolio( Ui::MonitorWindow* arg_m_pUi ) : m_pUi(arg_m_pUi), m_nRows(5)
{
}

Portfolio::~Portfolio()
{
}

void Portfolio::InitTable()
{
	// create sample data here
	// portfolio evaluation tab
	// common parameters
	/*
		S = 7000.000000
		r = 0.050000
		d = 0.000000
		FirstFixing = 1
		FixingsProJahr = 50
		AnzahlderDividende = 3
		n = 10000
	*/


	/* row parameters
		K = 89
		T = 0.515068
		sigma = 0.200000
	 */

	// Replace these with some random generated data
	//S, r, d, n, Sigma, FirstFixing, nAnzahlderDividende
	double S = 7000;
	double r = 0.05;;
	double d = 0;
	long n = 100000;
	double sigma = 0.2;
	int FirstFixing = 1;
	long AnzahlderDividende = 3;

	m_nRows = m_pUi->m_spinBox->value();
	m_pUi->m_calcSpreadSheet->setRowCount ( m_nRows );

	common_parameters_t comm_params(S, r, d, n, sigma, FirstFixing, AnzahlderDividende );

	arr_row_parameters_t arr_row_params;

	for( int k=0; k<m_nRows; k++ )
	{
		double dT = RandInt(1,365)/365.0;
		double dK = RandInt(80, 120);
		double dF = 50;

		arr_row_params.push_back(row_parameters_t(k, dT, dK, dF ));
	}

	InitPortfolio(comm_params, arr_row_params);

	m_pUi->m_progressBar->setRange(0, m_nRows-1);
	m_pUi->m_progressBar->reset();
}

void Portfolio::InitPortfolio( common_parameters_t& common_params, arr_row_parameters_t& v_row_params )
{
	//QComboBox
	//m_pUi->m_comboMethod.

	QString qstrUnd = "DAX";
	m_pUi->m_editUnderlying->setText(qstrUnd);

	//QLineEdit
	QString qstrCurr = "EUR";
	m_pUi->m_editCurrency->setText(qstrCurr);

	//QLineEdit
	QString qstrOrch = "Orchestrator";
	m_pUi->m_editOrchestrator->setText(qstrOrch);

	//QLineEdit
	//QString qstrSplitFact = "5";
	//m_pUi->m_editSplitFactor->setText(qstrSplitFact);

	// common parameters
	//QLineEdit
	QString qstrIntRate = QString("%1").arg(common_params.InterestRate());
	m_pUi->m_editIntRate->setText(qstrIntRate);

	//QLineEdit
	QString qstrSpotPrice = QString("%1").arg(common_params.SpotPrice());
	m_pUi->m_editSpotPrice->setText(qstrSpotPrice);

	//QLineEdit
	QString qstrDividendYield = QString("%1").arg(common_params.DividendYield());
	m_pUi->m_editDividendYield->setText(qstrDividendYield);

	//QLineEdit
	QString qstrIterations = QString("%1").arg(common_params.Iterations());
	m_pUi->m_editIterationsNb->setText(qstrIterations);

	//QLineEdit
	QString qstrVolatility = QString("%1").arg(common_params.Volatility());
	m_pUi->m_editVolatility->setText(qstrVolatility);

	for(int row_idx =0; row_idx <v_row_params.size(); row_idx++)
	{
		QTableWidgetItem *pWItem(new QTableWidgetItem( QString("Option %1").arg(row_idx) ));
		m_pUi->m_calcSpreadSheet->setVerticalHeaderItem( row_idx, pWItem );

		row_parameters_t& curr_row_params = v_row_params[row_idx];
		for( int col_idx = MATURITY; col_idx<=FIXINGS; col_idx++)
		{
			m_pUi->m_calcSpreadSheet->horizontalHeader()->resizeSection( row_idx, 71 );
			QTableWidgetItem *pItem(new QTableWidgetItem( QString("%1").arg(curr_row_params(col_idx), 10, ' ')) );
			pItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			m_pUi->m_calcSpreadSheet->setItem(row_idx, col_idx, pItem);
		}

		for( int col_idx = PV; col_idx<=VEGA; col_idx++)
			m_pUi->m_calcSpreadSheet->horizontalHeader()->resizeSection( row_idx, 100 );
	}

	//QLineEdit
	m_pUi->m_editTotalValue->setText("0");

	//QLineEdit
	m_pUi->m_editTotalDelta->setText("0");

	//QLineEdit
	m_pUi->m_editTotalGamma->setText("0");

	//QLineEdit
	m_pUi->m_editTotalVega->setText("0");
}

int Portfolio::Resize(int k)
{
	ClearTable();
	m_nRows = k;
	InitTable();
}

void Portfolio::ShowResult( simulation_result_t& sim_res )
{
	int row_idx = sim_res.rowId();

	/*m_pUi->m_calcSpreadSheet->horizontalHeader()->resizeSection( row_idx, 100 );

	QTableWidgetItem *pWItem(new QTableWidgetItem( QString("Option %1").arg(row_idx) ));
	m_pUi->m_calcSpreadSheet->setVerticalHeaderItem( row_idx, pWItem );*/

	//update fields PV, STDDEV, DELTA, GAMMA, VEGA
	for( int col_idx = PV; col_idx<=VEGA; col_idx++)
	{
		QTableWidgetItem *pItem(new QTableWidgetItem( QString("%1").arg(sim_res(col_idx), 12, ' ')) );
		pItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_pUi->m_calcSpreadSheet->setItem(row_idx, col_idx, pItem);
	}

	simulation_result_t::gTotalPV 	 += sim_res.Value();
	m_pUi->m_editTotalValue->setText(QString("%1").arg(simulation_result_t::gTotalPV ));

	simulation_result_t::gTotalDelta += sim_res.Delta();
	m_pUi->m_editTotalDelta->setText(QString("%1").arg(simulation_result_t::gTotalDelta));

	simulation_result_t::gTotalGamma += sim_res.Gamma();
	m_pUi->m_editTotalGamma->setText(QString("%1").arg(simulation_result_t::gTotalGamma));

	simulation_result_t::gTotalVega  += sim_res.Vega();
	m_pUi->m_editTotalVega->setText(QString("%1").arg(simulation_result_t::gTotalVega));
}

void Portfolio::ShowResults( arr_simulation_results_t& arr_sim_res )
{
	for(arr_simulation_results_t::iterator it = arr_sim_res.begin(); it != arr_sim_res.end(); it++ )
		ShowResult(*it);
}

// save the content of the worksheer into a job_data object
void Portfolio::PrepareInputData( portfolio_data_t& job_data  )
{
	// get first the common parameters
	QString qstr;
	qstr = m_pUi->m_editIntRate->text();
	job_data.common_params.setInterestRate(qstr.toDouble());

	//QLineEdit
	qstr = m_pUi->m_editSpotPrice->text();
	job_data.common_params.setSpotPrice(qstr.toDouble());

	//QLineEdit
	qstr = m_pUi->m_editDividendYield->text();
	job_data.common_params.setDividendYield(qstr.toDouble());

	//QLineEdit
	qstr = m_pUi->m_editIterationsNb->text();
	job_data.common_params.setIterations(qstr.toDouble());

	//QLineEdit
	qstr = m_pUi->m_editVolatility->text();
	job_data.common_params.setVolatility(qstr.toDouble());

	// these are always fixed
	job_data.common_params.setFirstFixing(1);
	job_data.common_params.setNumberDividends(3);

	// get now the specific parameters
	int m_nRows = m_pUi->m_calcSpreadSheet->rowCount();
	for(int row_idx =0; row_idx <m_nRows; row_idx++)
	{
		row_parameters_t row_params;
		for( int col_idx = MATURITY; col_idx<=FIXINGS; col_idx++)
		{
			QTableWidgetItem *pItem = m_pUi->m_calcSpreadSheet->item ( row_idx, col_idx );
			row_params(col_idx) = pItem->text().toDouble();
		}

		job_data.arr_row_params.push_back(row_params);
	}
}

void Portfolio::RetrieveResults( portfolio_result_t& result_data  )
{
}

static we::token_t make_token( portfolio_data_t& job_data, const int row )
{
  signature::structured_t sig;
  sig["bin"]=literal::STRING();
  sig["S"]=literal::DOUBLE();
  sig["sigma"]=literal::DOUBLE();
  sig["r"]= literal::DOUBLE();
  sig["d"]= literal::DOUBLE();
  sig["FirstFixing"] = literal::LONG();
  sig["AnzahlderDividende"]=  literal::LONG();
  sig["n"] = literal::LONG();

  sig["epsilon"]= literal::DOUBLE(); // TODO
  sig["delta"]=  literal::DOUBLE(); // TODO
  sig["iterations_per_run"]=  literal::LONG(); // TODO: figure out what this number actually means

  sig["rowID"] =  literal::LONG();
  sig["T"]=literal::DOUBLE();
  sig["K"]=literal::DOUBLE();
  sig["FixingsProJahr"]=literal::DOUBLE();

  value::structured_t param;
  param["bin"]= std::string("/amd/nfs/root/gpfs/p/hpc/sdpa/rotaru/sdpa_trunk/main/trunk/build_release/bin/Asian"); // TODO: update / make user chooseable
  param["S"]= job_data.common_params.SpotPrice();
  param["sigma"]= job_data.common_params.Volatility();
  param["r"]=job_data.common_params.InterestRate();
  param["d"]=job_data.common_params.DividendYield();
  param["FirstFixing"] = (long)job_data.common_params.FirstFixing();
  param["AnzahlderDividende"]= (long)job_data.common_params.NumberDividends();
  param["n"] = (long)job_data.common_params.Iterations();

  param["epsilon"]= 0.01; // TODO
  param["delta"]= 0.01; // TODO
  param["iterations_per_run"]= job_data.common_params.Iterations() / job_data.common_params.nLBUs(); // TODO: figure out what this number actually means

  param["rowID"] = literal::type((long)row);
  param["T"]= job_data.arr_row_params[row].Maturity();
  param["K"]= job_data.arr_row_params[row].Strike();
  param["FixingsProJahr"]=job_data.arr_row_params[row].Fixings();

  return we::token_t ( "param", sig, value::type(param) );
}

std::string Portfolio::BuildWorkflow(portfolio_data_t& job_data)
{
	// effectively build the flow here !!!!

	// 1. load xml file (job description/workflow) -> we::activity
	we::activity_t act;
	std::ifstream ifs("asian.pnet"); // TODO: make this configurable: file open dialog
	we::util::text_codec::decode (ifs, act);
	for (std::size_t row (0); row < job_data.size(); ++row)
		act.add_input( we::input_t::value_type( make_token(job_data, row), act.transition().input_port_by_name ("param")));

	std::ostringstream sstr;
	sstr << act;
	qDebug()<<"The workflow to be submitted is: " << QString(sstr.str().c_str());

	return we::util::text_codec::encode(act);
}

std::string Portfolio::BuildTestWorkflow(portfolio_data_t& job_data)
{
	std::string strJobData;
	job_data.PrintToString(strJobData);

	qDebug()<<"The workflow to be submitted is: ";
	qDebug()<<QString(strJobData.c_str());

	strJobData = job_data.encode();

	return strJobData;
}

void Portfolio::SubmitPortfolio()
{
	const int NMAXTRIALS = 10;
	int mPollingInterval(30000) ; //30 microseconds

	portfolio_data_t job_data;
	PrepareInputData( job_data  );

	// call here the client API
	// disable submit button

	// enable submit button when the job result is delivered (token)

	qDebug()<<"Starting the user client ...";
	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	std::ostringstream oss;
	oss<<"--orchestrator=orchestrator";
	cav.push_back(oss.str());
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );
	sdpa::job_id_t job_id_user;

	//std::string strWorkflow = BuildTestWorkflow(job_data);
	std::string strWorkflow = BuildWorkflow(job_data);

	int nTrials = 0;
	try {

		qDebug()<<"Submitting the workflow "<<strWorkflow.c_str();
		ClearTable();
		job_id_user = ptrCli->submitJob(strWorkflow);
		qDebug()<<"Got the job id "<<job_id_user.str().c_str();
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		if(nTrials++ > NMAXTRIALS)
		{
			qDebug()<<"The maximum number of job submission  trials was exceeded. Giving-up now!";

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;
		}
	}

	/*std::string job_status = ptrCli->queryJob(job_id_user);
	//qDebug()<<"The status of the job "<<job_id_user<<" is "<<job_status);
	std::cout<<std::endl;

	nTrials = 0;
	while( job_status.find("Finished") 	== std::string::npos &&
		   job_status.find("Failed") 	== std::string::npos &&
		   job_status.find("Cancelled") == std::string::npos )
	{
		try {
			job_status = ptrCli->queryJob(job_id_user);
			//qDebug()<<"The status of the job "<<job_id_user<<" is "<<job_status);
			std::cout<<".";
			boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			//qDebug()<<"Exception: "<<cliExc.what());
			std::cout<<"-";
			if(nTrials++ > NMAXTRIALS)
			{
				qDebug()<<"The maximum number of job submission  trials was exceeded. Giving-up now!";

				ptrCli->shutdown_network();
				ptrCli.reset();
				return;
			}

			boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
		}
	}

	qDebug()<<"The status of the job "<<job_id_user.str().c_str()<<" is "<<job_status.c_str();

	std::cout<<std::endl;
	nTrials = 0;
	try {
			qDebug()<<"Retrieve results of the job "<<job_id_user.str().c_str();
			ptrCli->retrieveResults(job_id_user);
			boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		if(nTrials++ > NMAXTRIALS)
		{
			qDebug()<<"The maximum number of job submission  trials was exceeded. Giving-up now!";

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;
		}

		boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
	}

	// reset trials counter
	nTrials = 0;
	try {
		qDebug()<<"Delete the user job "<<job_id_user.str().c_str();
		ptrCli->deleteJob(job_id_user);
		boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		if(nTrials++ > NMAXTRIALS)
		{
			qDebug()<<"The maximum number of job submission  trials was exceeded. Giving-up now!";

			ptrCli->shutdown_network();
			ptrCli.reset();
			return;
		}

		boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
	}

	ptrCli->shutdown_network();
	*/

}

void Portfolio::ClearTable( )
{
	for(int row_idx=0; row_idx<m_nRows; row_idx++ )
		//update fields PV, STDDEV, DELTA, GAMMA, VEGA
		for( int col_idx = PV; col_idx<=VEGA; col_idx++)
		{
			QTableWidgetItem *pItem(new QTableWidgetItem( QString("")) );
			m_pUi->m_calcSpreadSheet->setItem(row_idx, col_idx, pItem);
		}

	simulation_result_t::gTotalPV 	 = 0.0;
	simulation_result_t::gTotalDelta = 0.0;
	simulation_result_t::gTotalGamma = 0.0;
	simulation_result_t::gTotalVega  = 0.0;

	m_pUi->m_editTotalValue->setText("0");
	m_pUi->m_editTotalDelta->setText("0");
	m_pUi->m_editTotalGamma->setText("0");
	m_pUi->m_editTotalVega->setText("0");

	m_pUi->m_progressBar->reset();
}

void Portfolio::PrintToString(portfolio_data_t& d, std::string& strJobData)
{
	std::stringstream sstr;
	sstr<<"S = "<<d.common_params.SpotPrice()<<std::endl;
	sstr<<"r = "<<d.common_params.InterestRate()<<std::endl;
	sstr<<"d = "<<d.common_params.DividendYield()<<std::endl;
	sstr<<"n = "<<d.common_params.Iterations()<<std::endl;
	sstr<<"sigma = "<<d.common_params.Volatility()<<std::endl;
	sstr<<"FirstFixing = "<<d.common_params.FirstFixing()<<std::endl;
	sstr<<"AnzahlderDividende = "<<d.common_params.NumberDividends()<<std::endl;

	for( int row_idx = 0; row_idx<1/*d.arr_row_params.size()*/; row_idx++ )
	{
		sstr<<"row = "<<row_idx<<std::endl;
		sstr<<"T = "<<d.arr_row_params[row_idx].Maturity()<<std::endl;
		sstr<<"K ="<<d.arr_row_params[row_idx].Strike()<<std::endl;
		sstr<<"FixingsProJahr = "<<d.arr_row_params[row_idx].Fixings()<<std::endl;
	}

	strJobData = sstr.str();
}

