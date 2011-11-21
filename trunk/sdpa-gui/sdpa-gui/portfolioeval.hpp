#ifndef PORTFOLIO_HPP
#define PORTFOLIO_HPP

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include <QWidget>

#include <sdpa/client/ClientApi.hpp>
#include <apps/portfolio_params.hpp>
#include <string>

namespace Ui {
    class MonitorWindow;
}

class Portfolio : public QWidget {
  Q_OBJECT
  
public:
	Portfolio( Ui::MonitorWindow* );
	virtual ~Portfolio();
        void Init();
	void InitTable();
	void InitPortfolio( common_parameters_t&, arr_row_parameters_t& );
	void ShowResult( simulation_result_t& );
	void ShowResults( arr_simulation_results_t& );
	void PrepareInputData( portfolio_data_t& );
	void PrintToString(portfolio_data_t&, std::string& );
	std::string BuildWorkflow(portfolio_data_t&);
	std::string BuildTestWorkflow(portfolio_data_t& job_data);

	void ClearTable();
	void SubmitPortfolio();
	int  RandInt(int low, int high);
	int  Resize(int k);
  void StartClient();
  void StopClient();
	void WaitForCurrJobCompletion();
	void Poll();
	void EnableControls();
	void DisableControls();

  bool event (QEvent *event);
private:
	Ui::MonitorWindow *m_pUi;
	int m_nRows;
	sdpa::client::ClientApi::ptr_t m_ptrCli;
	sdpa::job_id_t m_currentJobId;
	bool m_bClientStarted;

  boost::shared_ptr<boost::thread> m_poll_thread;
};

#endif
