#ifndef PORTFOLIO_HPP
#define PORTFOLIO_HPP

#include <apps/portfolio_params.hpp>
#include <string>

namespace Ui {
    class MonitorWindow;
}

class Portfolio {
public:
	Portfolio( Ui::MonitorWindow* );
	virtual ~Portfolio();
	void InitTable();
	void InitPortfolio( common_parameters_t&, arr_row_parameters_t& );
	void ShowResult( simulation_result_t& );
	void ShowResults( arr_simulation_results_t& );
	void PrepareInputData( portfolio_data_t& );
	void RetrieveResults( portfolio_result_t& );
	void PrintToString(portfolio_data_t&, std::string& );
	std::string BuildWorkflow(portfolio_data_t&);

	void ClearTable();
	void SubmitPortfolio();
	int  RandInt(int low, int high);

private:
	Ui::MonitorWindow *m_pUi;
	int m_nRows;
};

#endif
