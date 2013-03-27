
#ifndef CREATE_DRTS_WORKER_HPP
#define CREATE_DRTS_WORKER_HPP 1

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

sdpa::shared_ptr<fhg::core::kernel_t> createDRTSWorker(	const std::string& drtsName,
														const std::string& masterName,
														const std::string& cpbList,
														const std::string& strModulesPath,
														const std::string& kvsHost,
														const std::string& kvsPort
												)
{
	sdpa::shared_ptr<fhg::core::kernel_t> kernel(new fhg::core::kernel_t);
    kernel->set_name (drtsName);

	kernel->put("plugin.kvs.host", kvsHost);
	kernel->put("plugin.kvs.port", kvsPort);

	//see ~/.sdpa/configs/sdpa.rc
	std::string guiUrl("localhost:6408");
	//kernel->put("plugin.gui.url", guiUrl);

	kernel->put("plugin.drts.name", drtsName);
	kernel->put("plugin.drts.master", masterName);
	kernel->put("plugin.drts.backlog", "2");
	kernel->put("plugin.drts.request-mode", "false");

	if(!cpbList.empty())
		kernel->put("plugin.drts.capabilities", cpbList);

	kernel->put("plugin.wfe.library_path", strModulesPath);

	kernel->load_plugin (TESTS_KVS_PLUGIN_PATH);
	//kernel->load_plugin (TESTS_GUI_PLUGIN_PATH);
	kernel->load_plugin (TESTS_WFE_PLUGIN_PATH);
	kernel->load_plugin (TESTS_FVM_FAKE_PLUGIN_PATH);
	kernel->load_plugin (TESTS_DRTS_PLUGIN_PATH);

	return kernel;
}

#endif
