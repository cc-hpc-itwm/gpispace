// KDM include files
#include "structures/migrationjob.h"
#include "filehandler/checkreadmigrationjob.h"
#include "filehandler/migrationfilehandler.h"
#include "TraceBunch.hpp"
#include "MigSubVol.hpp"
#include "sdpa_migrate.hpp"

#include "ttvmmemhandler.h"

#include "sinc_mod.hpp"

int main ()
{
  std::cout << sizeof (MigrationJob) << std::endl;

  return EXIT_SUCCESS;
}
