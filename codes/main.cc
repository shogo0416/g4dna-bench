/*==============================================================================
  BSD 2-Clause License

  Copyright (c) 2020-2022 Shogo OKADA (shogo.okada@kek.jp)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================*/
#include "globals.hh"

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "application.h"
#include "timehistory.h"
#include "simdata.h"

#include <getopt.h>
#include <string>

namespace {

//------------------------------------------------------------------------------
void print_usage()
{

  const char* usage = R"(
    [Usage] chem-bench <options>
    [Options]
      -h, --help               print this information
      -c, --conf   <file_name> set configuration file [defualt: conf.json]
      -s, --seed   <val>       set seed for random number generation
      -o, --output <file_name> set output file name
  )";

  std::cout << usage << std::endl;

  std::exit(EXIT_SUCCESS);
}

} // end of anonymous namespace

//==============================================================================
// main function
int main(int argc, char** argv)
{

  struct option opts [] = {
    {"help",   no_argument,       nullptr, 'h'},
    {"conf",   required_argument, nullptr, 'c'},
    {"seed",   required_argument, nullptr, 's'},
    {"output", required_argument, nullptr, 'o'},
    {nullptr,  0,                 nullptr,  0},
  };

  int seed = -1;
  std::string conf_file   = "conf.json";
  std::string output_file = "";

  const char* optstr = "hc:s:o:";
  int opt, index;
  while ((opt = getopt_long(argc, argv, optstr, opts, &index)) != -1) {
    switch (opt) {
      case 'h':
        ::print_usage();
        break;
      case 'c':
        conf_file = static_cast<std::string>(optarg);
        break;
      case 's':
        seed = atoi(optarg);
        break;
      case 'o':
        output_file = static_cast<std::string>(optarg);
        break;
    }
  }

  auto app = Application::GetInstance();

  app->LoadConfigFile(conf_file);

  if (output_file.length() > 0) { app->SetOutputFile(output_file); }

  app->SetupRandomEngine(seed);

#ifdef G4MULTITHREADED
  auto run = new G4MTRunManager();
#else
  auto run = new G4RunManager();
#endif

  app->Setup();

  run->SetUserInitialization(app);

  // initialization
  run->Initialize();

  auto timer = TimeHistory::GetTimeHistory();

  // start simulation
  timer->ShowClock("[MESSAGE] Start:");
  timer->TakeSplit("BeamOn");

  run->BeamOn(app->GetEventNumber());

  timer->TakeSplit("BeamEnd");
  timer->ShowClock("[MESSAGE] End:");

  auto sd = SimData::GetInstance();
  sd->SaveSimulationResult();
  sd->SaveBenchmarkResult();

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted
  // in the main() program !
  delete run;

  std::exit(EXIT_SUCCESS);
}
