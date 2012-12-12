#include "monitorwindow.hpp"
#include "portfolioeval.hpp"
#include "ui_monitorwindow.h"
#include <QDebug>

double simulation_result_t::gTotalPV 	= 0.0;
double simulation_result_t::gTotalDelta = 0.0;
double simulation_result_t::gTotalGamma = 0.0;
double simulation_result_t::gTotalVega 	= 0.0;

#include <vector>
#include <sstream>
#include <QtGlobal>
#include <QTime>

#include <QMessageBox>
#include <QDir>

#include <sdpa/engine/IWorkflowEngine.hpp>
#include <boost/tokenizer.hpp>
#include <fhgcom/kvs/kvsc.hpp>

#include <we/type/token.hpp>

static int enable_disable_event_type = QEvent::registerEventType();

class Enable_Disable_Controls : public QEvent
{
public:
  explicit
  Enable_Disable_Controls(bool what)
    : QEvent((QEvent::Type)(enable_disable_event_type))
    , m_what (what)
  {}

  bool is_enable () const { return m_what; }
private:
  bool m_what;
};

static void postEnableControls(QObject *receiver)
{
  QCoreApplication::postEvent ( receiver
				, new Enable_Disable_Controls (true)
				);
}

const int NMAXTRIALS = 10;
int mPollingInterval(1000000) ; //1 second

int Portfolio::RandInt(int low, int high)
{
	// Random number between low and high
	return qrand() % ((high + 1) - low) + low;
}

Portfolio::Portfolio( Ui::MonitorWindow* arg_m_pUi ) : m_pUi(arg_m_pUi), m_nRows(5), m_bClientStarted(false)
{
}

Portfolio::~Portfolio()
{
}

void Portfolio::Init()
{
  QString strBackendDir("/usr/local");
  char *sdpa_home = std::getenv("SDPA_LIBEXEC");
  if (sdpa_home)
  {
    strBackendDir = sdpa_home;
  }

  m_pUi->m_editBackendFile->setText(strBackendDir + "/apps/asian/bin/Asian");
  m_pUi->m_editWorkflowFile->setText(strBackendDir + "/apps/asian/asian.pnet");
  m_pUi->m_nThreads->setValue(1);

	char* szKvsEnv = std::getenv("KVS_URL");
	if(szKvsEnv)
	{
		std::string strKvsUrl(szKvsEnv);

		boost::char_separator<char> sep(":");
		boost::tokenizer<boost::char_separator<char> > tok(strKvsUrl, sep);

		std::vector< std::string > vec;
		vec.assign(tok.begin(),tok.end());

		if( vec.size() == 2 )
			m_pUi->m_editKvsUrl->setText(szKvsEnv);
		else
			m_pUi->m_editKvsUrl->setText("localhost:2439");
	}
	else
	{
		m_pUi->m_editKvsUrl->setText("localhost:2439");
	}
}

void Portfolio::InitTable()
{
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
		double dT = RandInt(100,365)/365.0;
		double dK = RandInt(80, 120);
		double dF = 50;

		arr_row_params.push_back(row_parameters_t(k, dT, dK, dF ));
	}

	InitPortfolio(comm_params, arr_row_params);
}

void Portfolio::InitPortfolio( common_parameters_t& common_params, arr_row_parameters_t& v_row_params )
{
  m_pUi->m_progressBar->setRange(0, 4 * v_row_params.size() * common_params.nLBUs() + v_row_params.size() - 1);
  m_pUi->m_progressBar->reset();

	//QComboBox
	//m_pUi->m_comboMethod.

	QString qstrUnd = "DAX";
	m_pUi->m_editUnderlying->setText(qstrUnd);

	//QLineEdit
	QString qstrCurr = "EUR";
	m_pUi->m_editCurrency->setText(qstrCurr);

	//QLineEdit
	QString qstrOrch = "orchestrator";
	m_pUi->m_editOrchestrator->setText(qstrOrch);

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

	for( arr_row_parameters_t::size_type row_idx (0)
           ; row_idx < v_row_params.size()
           ; ++row_idx
           )
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
	m_pUi->m_editTotalValue->setEnabled(false);
	m_pUi->m_editTotalValue->setText("0");

	//QLineEdit
	m_pUi->m_editTotalDelta->setEnabled(false);
	m_pUi->m_editTotalDelta->setText("0");

	//QLineEdit
	m_pUi->m_editTotalGamma->setEnabled(false);
	m_pUi->m_editTotalGamma->setText("0");

	//QLineEdit
	m_pUi->m_editTotalVega->setEnabled(false);
	m_pUi->m_editTotalVega->setText("0");
}

int Portfolio::Resize(int k)
{
	ClearTable();
	m_nRows = k;
	InitTable();
        return -1;
}

void Portfolio::ShowResult( simulation_result_t& sim_res )
{
	int row_idx = sim_res.rowId();

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

static token::type make_token( const QString& qstrBackend, portfolio_data_t& job_data, const int row, const long nThreads )
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
  sig["nThreads"] = literal::LONG();

  value::structured_t param;

  param["bin"]= qstrBackend.toStdString(); // TODO: update / make user chooseable
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
  param["nThreads"]=nThreads;

  return token::type ( "param", sig, value::type(param) );
}

std::string Portfolio::BuildWorkflow(portfolio_data_t& job_data)
{
	QString qstrBackend = m_pUi->m_editBackendFile->text();

	QFile wfFile(qstrBackend);
	if( !wfFile.exists() )
	{
		QMessageBox::critical( 	m_pUi->SDPAGUI,
								QString("Error!"),
								QString("There is no file named ") + qstrBackend
							 );
		EnableControls();
		return "";
	}

	std::ifstream ifs(m_pUi->m_editWorkflowFile->text().toStdString().c_str()); // TODO: make this configurable: file open dialog

	if(!ifs.good())
	{
		QMessageBox::critical( m_pUi->SDPAGUI,
								QString("Error!"),
								QString("Could not open the workflow file  ") + m_pUi->m_editWorkflowFile->text()  + QString(". Giving up now!")
							);
		EnableControls();
		return "";
	}


	we::activity_t act;

        try
        {
          we::util::codec::decode (ifs, act);
        }
        catch (std::exception const & ex)
        {
          QMessageBox::critical( m_pUi->SDPAGUI
                               , QString("Error!")
                               , QString("Could not load workflow: ") + QString(ex.what())
                               );
          EnableControls();
          return "";
        }

        try
        {
          for (std::size_t row (0); row < job_data.size(); ++row)
            act.add_input( we::input_t::value_type
                         ( make_token
                         ( qstrBackend
                         , job_data
                         , row
                         , m_pUi->m_nThreads->value()
                         )
                         , act.transition().input_port_by_name ("param")
                         )
                         );
        }
        catch (std::exception const & ex)
        {
          QMessageBox::critical( m_pUi->SDPAGUI
                               , QString("Error!")
                               , QString("Could not place tokens: ") + QString(ex.what())
                               );
          EnableControls();
          return "";
        }

	ifs.close();
	return we::util::codec::encode(act);
}

void Portfolio::StopClient()
{
  try
  {
    if (! m_bClientStarted)
    {
      return;
    }

    if ( m_poll_thread
      && (boost::this_thread::get_id() != m_poll_thread->get_id())
       )
    {
      m_poll_thread->interrupt();
      m_poll_thread->join();
    }

    m_ptrCli->shutdown_network();
    m_ptrCli.reset();
    m_bClientStarted = false;
  }
  catch (std::exception const & ex)
  {
    //    qDebug () << "Could not stop client: " << ex.what();
    m_bClientStarted = false;
  }
}

void Portfolio::StartClient()
{
	if (m_bClientStarted)
		return;


	std::string strKvsUrl = m_pUi->m_editKvsUrl->text().toStdString();

	boost::char_separator<char> sep(":");
	boost::tokenizer<boost::char_separator<char> > tok(strKvsUrl, sep);

	std::vector< std::string > vec;
	vec.assign(tok.begin(),tok.end());

	if( vec.size() != 2 )
	{
		QMessageBox::critical( 	m_pUi->SDPAGUI,
								QString("Error!"),
								QString("Invalid kvs url. Please specify it in the form <hostname (IP)>:<port>!")
							);
		return;
	}
	else
	{
	    //check first if the fhgkvsd can be reached
		qDebug()<<"The kvs daemon is assumed to run at "<<vec[0].c_str()<<":"<<vec[1].c_str();
		try
		{
                  fhg::com::kvs::global::get_kvs_info().init( vec[0]
                                                            , vec[1]
                                                            , boost::posix_time::seconds(1)
                                                            , 1
                                                            );
                  fhg::com::kvs::put("test_val", 42);
                  fhg::com::kvs::del("test_val");
		}
		catch(std::exception const & ex)
		{
                  QMessageBox::critical( m_pUi->SDPAGUI
                                       , "Error!"
                                       , QString("Invalid url. The kvs daemon could not be contacted at ") + strKvsUrl.c_str() + ": " + ex.what()
                                       );

                  m_bClientStarted = false;
                  return;
		}
	}

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	qDebug()<<"Starting the user client with the following parameters:";
	std::vector<std::string> cav;
	std::ostringstream oss;

	QString qstrOrch = m_pUi->m_editOrchestrator->text();
	oss<<"--orchestrator="<<qstrOrch.toStdString();
	cav.push_back(oss.str());
	qDebug()<<oss.str().c_str();

	config.parse_command_line(cav);

  try {
    m_ptrCli = sdpa::client::ClientApi::create( config, "sdpa-gui:"+boost::lexical_cast<std::string>(getpid()));
    m_ptrCli->configure_network( config );

    m_bClientStarted = true;
  }
  catch(const std::exception& ex)
  {
	  qDebug()<<"Exception occurred: "<<ex.what();
	  QMessageBox::critical( m_pUi->SDPAGUI,
						   	 QString("Portfolio evaluation"),
						   	 QString("The client could not be started! Exception occurred: "+ QString(ex.what())) );

          StopClient();
	  //throw ex;
  }
}

void Portfolio::Poll()
{
  std::string job_status ("UNKNONW");

  int nTrials = 0;
  while( job_status.find("Finished") 	== std::string::npos &&
       job_status.find("Failed") 	== std::string::npos &&
       job_status.find("Cancelled") == std::string::npos )
  {
    try {
      job_status = m_ptrCli->queryJob(m_currentJobId);
      nTrials = 0;
      boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
    }
    catch(const std::exception& cliExc)
    {
      qDebug()<<"Exception occured: "<<cliExc.what();
      if(nTrials++ > NMAXTRIALS)
      {
        qDebug()<<"The maximum number of job submission  trials was exceeded. Giving-up now!";
        StopClient();
	postEnableControls(this);
        return;
      }

      boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
    }
  }

  qDebug()<<"The status of the job "<<m_currentJobId.str().c_str()<<" is "<<job_status.c_str();

  try {
    qDebug()<<"Retrieve results of the job "<<m_currentJobId.str().c_str();
    m_ptrCli->retrieveResults(m_currentJobId);
    m_ptrCli->deleteJob(m_currentJobId);
  }
  catch(const std::exception& cliExc)
  {
    qDebug() << "could not delete job: " << cliExc.what();
    StopClient();
  }

  postEnableControls(this);
}

bool Portfolio::event (QEvent *event)
{
  if (event->type() == enable_disable_event_type)
    {
      EnableControls();
      event->accept();
      return true;
    }
  return QWidget::event(event);
}

void Portfolio::WaitForCurrJobCompletion()
{
  if (m_poll_thread)
  {
    m_poll_thread->interrupt();
    m_poll_thread->join();
  }

  m_poll_thread.reset(new boost::thread(boost::bind(&Portfolio::Poll, this)));
}

void Portfolio::DisableControls()
{
	m_pUi->m_submitButton->setEnabled(false);
	m_pUi->m_clearButton->setEnabled(false);
	m_pUi->m_spinBox->setEnabled(false);
}

void Portfolio::EnableControls()
{
	m_pUi->m_submitButton->setEnabled(true);
	m_pUi->m_clearButton->setEnabled(true);
	m_pUi->m_spinBox->setEnabled(true);
}

void Portfolio::SubmitPortfolio()
{
  if (m_poll_thread)
  {
    m_poll_thread->interrupt();
    m_poll_thread->join();
  }

  /*
	if (m_bClientStarted)
	{
		StopClient();
	}
  */

	if(!m_bClientStarted)
	{
		try{
			StartClient();
		}
		catch(const std::exception& ex)
		{
			m_bClientStarted = false;
			qDebug()<<"The client could not be started. The following exception occurred: "<<ex.what();

			QMessageBox::critical( m_pUi->SDPAGUI,
							   QString("Portfolio evaluation"),
							   QString("The client could not be started! Exception occurred: "+ QString(ex.what())) );
			return;
		}
	}

	if( m_bClientStarted )
	{
		portfolio_data_t job_data;
		PrepareInputData( job_data  );

		// call here the client API
		// disable submit button
		std::string strWorkflow = BuildWorkflow(job_data);

		if(strWorkflow.empty())
			return;

		int nTrials = 0;
		try {
                  //			qDebug()<<"Submitting the workflow "<<strWorkflow.c_str();
			ClearTable();
			DisableControls();
			m_currentJobId = m_ptrCli->submitJob(strWorkflow);
                        //	qDebug()<<"Got the job id "<<m_currentJobId.str().c_str();
		}
		catch(const std::exception& cliExc)
		{
                  //m_ptrCli.reset();
                  QMessageBox::critical( m_pUi->SDPAGUI,
                                       QString("Exception!"),
                                       QString(cliExc.what())
                                       );
                  StopClient();
                  EnableControls();
                  return;

		}

		WaitForCurrJobCompletion();
	}
}

void Portfolio::ClearTable( )
{
	for(int row_idx=0; row_idx<m_nRows; row_idx++ )
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

