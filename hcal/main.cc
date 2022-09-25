/*============================================================================
Copyright 2022 Koichi Murakami

Distributed under the OSI-approved BSD License (the "License");
see accompanying file LICENSE for details.

This software is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the License for more information.
============================================================================*/
#include <getopt.h>
#include <string>
#include "G4RunManagerFactory.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4UItcsh.hh"
#ifdef ENABLE_VIS
#include "G4VisExecutive.hh"
#endif

#include "version.h"
#include "common/appbuilder.h"
#include "common/g4environment.h"
#include "util/jsonparser.h"
#include "util/timehistory.h"

using namespace kut;

namespace {
// --------------------------------------------------------------------------
void show_version()
{
  const char* version_str = G4BENCH_VERSION_MAJOR "."
                            G4BENCH_VERSION_MINOR ".";

  std::cout << "G4Bench/hcal version 2.0.0"
            << " (" << version_str << ::build_head << "."
            << ::build_tail << ")" << std::endl;
}

// --------------------------------------------------------------------------
void show_help()
{
  const char* message =
R"(
usage:
hcal [options] [#histories]

   -h, --help          show this message.
   -v  --version       show program name/version.
   -c, --config        specify configuration file [g4bench.conf]
   -s, --session=type  specify session type [tcsh]
   -i, --init=macro    specify initial macro
   -n, --nthreads=N    set number of threads [1]
   -q, --serial        run in serial mode [false]
   -b, --bench=name    set benchmark name [hcal]
   -p, --cpu=name      set CPU name [unknown]
)";

   std::cout << message << std::endl;
}

} // end of namespace

// --------------------------------------------------------------------------
int main(int argc, char** argv)
{
  // optional parameters
  bool qhelp = false;
  bool qversion = false;
  std::string session_type = "tcsh";
  std::string init_macro = "";
  std::string config_file = "g4bench.conf";
  std::string str_nhistories = "";
  std::string str_nthreads = "1";
  bool qserial = false;
  std::string str_bench = "hcal";
  std::string str_cpu = "unknown";

  struct option long_options[] = {
    {"help",       no_argument,         0,  'h'},
    {"version",    no_argument,         0,  'v'},
    {"config",     required_argument,   0,  'c'},
    {"session",    required_argument,   0,  's'},
    {"init",       required_argument,   0,  'i'},
    {"nthreads",   required_argument,   0,  'n'},
    {"serial",     no_argument,         0,  'q'},
    {"bench",      required_argument,   0,  'b'},
    {"cpu",        required_argument,   0,  'p'},
    {0,            0,                   0,   0}
  };

  while (1) {
    int option_index = -1;

    int c = getopt_long(argc, argv, "hvc:s:i:n:qb:p:",
                        long_options, &option_index);

    if (c == -1) break;

    switch (c) {
    case 'h' :
      qhelp = true;
      break;
    case 'v' :
      qversion = true;
      break;
    case 'c' :
      config_file = optarg;
    case 's' :
      session_type = optarg;
      break;
    case 'i' :
      init_macro = optarg;
      break;
    case 'n' :
      str_nthreads = optarg;
      break;
    case 'q' :
      qserial = true;
      break;
    case 'b' :
      str_bench = optarg;
      break;
    case 'p' :
      str_cpu = optarg;
      break;
    default:
      std::exit(EXIT_FAILURE);
      break;
    }
  }

  if ( qhelp ) {
    ::show_version();
    ::show_help();
  }

  if ( qversion ) {
    ::show_version();
  }

  if ( qhelp || qversion ) {
    std::exit(EXIT_SUCCESS);
  }

  // #threads
  int nthreads = 1;
  try {
    nthreads = std::stoi(str_nthreads);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    std::cout << "[ ERROR ] invalid argument: <#threads>" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  if(nthreads <= 0 ) {
    std::cout << "[ ERROR ] #threads should be more than 0." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // #histories
  int nhistories = 0.;
  if ( optind < argc ) {
    str_nhistories = argv[optind];
    try {
      nhistories = std::stoi(str_nhistories);
    } catch (std::exception& e) {
      std::cout << e.what() << std::endl;
      std::cout << "[ ERROR ] invalid argument: <#histories>" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    if(nhistories <= 0 ) {
      std::cout << "[ ERROR ] #histories should be more than 0." << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  // load config
  auto jparser = JsonParser::GetJsonParser();
  bool qload = jparser-> LoadFile(config_file);
  if ( ! qload ) {
    std::cout << "[ ERROR ] failed on loading a config file. "
              << config_file << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // ----------------------------------------------------------------------
  std::cout << "=============================================================="
            << std::endl;
  ::show_version();
  std::cout << "   * config file = " << config_file << std::endl
            << "   * # of threads = " << nthreads << std::endl
            << "   * # of histories = " << nhistories
            << std::endl;
  std::cout << "=============================================================="
            << std::endl;

  std::cout << "JSON configuration" << std::endl;
  jparser-> DumpAll();
  std::cout << "=============================================================="
            << std::endl;

  std::cout << "G4DATA DIRs:" << std::endl;
  auto g4data_dir = jparser-> GetStringValue("Run/G4DATA");
  G4Environment::SetDataDir(g4data_dir);
  G4Environment::SetEnvironment();
  G4Environment::PrintEnvironment();
  std::cout << "=============================================================="
            << std::endl;

  // ----------------------------------------------------------------------
  auto gtimer = TimeHistory::GetTimeHistory();
  gtimer-> ShowClock("[MESSAGE] Start:");

  // G4 managers & setup application
  G4RunManager* run_manager { nullptr };
  if ( qserial ) {
    run_manager =
      G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);

  } else {
    run_manager =
      G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
    run_manager-> SetNumberOfThreads(nthreads);
  }

  auto ui_manager = G4UImanager::GetUIpointer();

  auto appbuilder = new AppBuilder();
  appbuilder-> SetTestingFlag(true, str_bench, str_cpu);
  appbuilder-> BuildApplication();

  // ----------------------------------------------------------------------
#ifdef ENABLE_VIS
  auto vis_manager = new G4VisExecutive("quiet");
  vis_manager-> Initialize();
#endif

  // do init macro
  if (init_macro != "" ) {
    std::string command = "/control/execute ";
    ui_manager-> ApplyCommand(command + init_macro);
  }

  // start session
  bool qbatch = nhistories > 0;
  if ( qbatch ) {
    gtimer-> TakeSplit("BeamOn");
    run_manager-> BeamOn(nhistories);
    gtimer-> TakeSplit("BeamEnd");

  } else {
    auto ui_session = new G4UIExecutive(argc, argv, session_type);
    gtimer-> TakeSplit("SessionStart");
    ui_session-> SetPrompt("hcal(%s)[%/]:");
    ui_session-> SessionStart();
    gtimer-> TakeSplit("SessionEnd");
    delete ui_session;
  }

  // ----------------------------------------------------------------------
#ifdef ENABLE_VIS
  delete vis_manager;
#endif

  delete run_manager;

  gtimer-> ShowClock("[MESSAGE] End:");

  return EXIT_SUCCESS;
}
