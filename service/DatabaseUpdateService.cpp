#include <boost/thread.hpp>

#include "DatabaseUpdateService.hpp"

DatabaseUpdateService::DatabaseUpdateService(const boost::filesystem::path& p)
: _metadataParser(),
 _databaseUpdater( p, _metadataParser)
{
}

void
DatabaseUpdateService::start(void)
{
	_databaseUpdater.start();
}

void
DatabaseUpdateService::stop(void)
{
	std::cout << "DatabaseUpdateService::stop, processing..." << std::endl;
	_databaseUpdater.stop();
	std::cout << "DatabaseUpdateService::stop, process done" << std::endl;
}

void
DatabaseUpdateService::restart(void)
{
	std::cout << "DatabaseUpdateService::restart" << std::endl;
	stop();
	start();
}
