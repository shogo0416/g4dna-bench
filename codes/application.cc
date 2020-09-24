/*==============================================================================
  BSD 2-Clause License

  Copyright (c) 2020 Shogo OKADA (shogo.okada@kek.jp)
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
#include "application.h"
#include "geometry.h"
#include "primary_generator.h"
#include "primary_removal.h"
#include "physics_list.h"
#include "event_action.h"
#include "run_action.h"
#include "stacking_action.h"
#include "simdata.h"
#include "step_action.h"
#include "time_step_action.h"
#include "tracking_interactivity.h"

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif
#include "G4MoleculeCounter.hh"
#include "G4DNAChemistryManager.hh"
#include "G4H2O.hh"
#include "G4DNAModelSubType.hh"
#include "G4EmParameters.hh"
#include "G4SystemOfUnits.hh"
#include "G4Version.hh"
#include "G4Scheduler.hh"
#include "CLHEP/Random/MTwistEngine.h"

#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

namespace {

static json js;

//------------------------------------------------------------------------------
void print_parameters()
{

  std::cout << "---------------------------------------------------------------"
            << std::endl;
  std::cout << " Parameters (g4dna-bench)" << std::endl;
  std::cout << "---------------------------------------------------------------"
            << std::endl;

  for (auto x : js.items()) {
    std::cout << " - " << x.key() << " : " << x.value() << std::endl;
  }

  std::cout << "---------------------------------------------------------------"
            << std::endl;
  std::cout << std::endl;
}

//------------------------------------------------------------------------------
void set_solvation_model(const std::string& name)
{
  G4DNAModelSubType type = fDNAUnknownModel;
  if (name == "Ritchie1994")
  {
    type = fRitchie1994eSolvation;
  }
  else if (name == "Terrisol1990")
  {
    type = fTerrisol1990eSolvation;
  }
  else if (name == "Meesungnoen2002")
  {
    type = fMeesungnoen2002eSolvation;
  }
#if G4VERSION_NUMBER >= 1060
  else if (name == "Meesungnoen2002_amorphous")
  {
    type = fMeesungnoensolid2002eSolvation;
  }
  else if (name == "Kreipl2009")
  {
    type = fKreipl2009eSolvation;
  }
#endif
  else
  {
    std::cerr << "[ERROR] Unknown model was set for electron solvation "
              << "process (name: " << name << ")" << std::endl;
    std::cerr << "--> Supported types models: "
              << "Ritchie1994, Terrisol1990, Meesungnoen2002"
#if G4VERSION_NUMBER >= 1060
              << ", Meesungnoen2002_amorphous, and Kreipl2009"
#endif
              << std::endl;
    std::exit(EXIT_FAILURE);
  }
  G4EmParameters::Instance()->SetDNAeSolvationSubType(type);
}

} // end of anonymous namespace

//==============================================================================

Application* Application::instance_ = nullptr;

//------------------------------------------------------------------------------
Application::Application()
    : G4VUserActionInitialization()
{
  primary_removal_ = false;
  kill_elow_ = DBL_MAX;
  kill_eupp_ = DBL_MAX;
}

//------------------------------------------------------------------------------
Application* Application::GetInstance()
{
  if (!instance_) { instance_ = new Application(); }
  return instance_;
}

//------------------------------------------------------------------------------
void Application::BuildForMaster() const
{
  SetUserAction(new RunAction());
  G4DNAChemistryManager::Instance()->ResetCounterWhenRunEnds(false);
}

//------------------------------------------------------------------------------
void Application::Build() const
{
  bool use = ::js["use_molecule_counter"];
  if (use) {
    G4MoleculeCounter::Use();
    G4MoleculeCounter::Instance()->DontRegister(G4H2O::Definition());
  }
  SimData::GetInstance()->Setup();

  if (!G4Threading::IsMultithreadedApplication()) {
    G4DNAChemistryManager::Instance()->ResetCounterWhenRunEnds(false);
  }

  auto pkind  = ::js["beam_particle"];
  auto Z      = ::js["beam_ion_Z"];
  auto A      = ::js["beam_ion_A"];
  auto energy = ::js["beam_energy"].get<double>() * keV;
  auto posx   = ::js["beam_source_pos"][0].get<double>() * um;
  auto posy   = ::js["beam_source_pos"][1].get<double>() * um;
  auto posz   = ::js["beam_source_pos"][2].get<double>() * um;
  auto dirx   = ::js["beam_direction"][0];
  auto diry   = ::js["beam_direction"][1];
  auto dirz   = ::js["beam_direction"][2];

  auto pgen = new PrimaryGenerator();
  pgen->SetParticle(pkind, Z, A);
  pgen->SetEnergy(energy);
  pgen->SetPosition(posx, posy, posz);
  pgen->SetDirection(dirx, diry, dirz);
  SetUserAction(pgen);

  SetUserAction(new StepAction());
  SetUserAction(new EventAction());
  SetUserAction(new RunAction());
  SetUserAction(new StackingAction());

  bool check_boundary = ::js["check_boundary"];
  auto tsa = new TimeStepAction();
  tsa->CheckBoundary(check_boundary);
  G4Scheduler::Instance()->SetUserAction(tsa);

  auto tri = new TrackingInteractivity();
  tri->SetSteppingAction(new StepAction(true));
  G4Scheduler::Instance()->SetInteractivity(tri);
}

//------------------------------------------------------------------------------
void Application::Setup(const std::string& conf_file)
{

#ifdef G4MULTITHREADED
  auto run = new G4MTRunManager();
#else
  auto run = new G4RunManager();
#endif

  std::ifstream fin(conf_file.c_str());
  if (fin.fail()) {
    std::cerr << "[ERROR] Failed to open configuration file (name: "
              << conf_file << ")" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  fin >> ::js;
  ::print_parameters();

  // setup event number processing and thread number
  num_event_  = ::js["event_number"];
  num_thread_ = ::js["thread_number"];

  // setup event number processing, thread number, and seed
  auto seed = ::js["random_seed"];
  G4Random::setTheEngine(new CLHEP::MTwistEngine);
  G4Random::setTheSeed(seed);

  // setup water phantom
  auto target_size_x = ::js["target_size"][0].get<double>() * um;
  auto target_size_y = ::js["target_size"][1].get<double>() * um;
  auto target_size_z = ::js["target_size"][2].get<double>() * um;

  auto geom = Geometry::GetInstance();
  geom->SetPhantomSize(target_size_x, target_size_y, target_size_z);
  run->SetUserInitialization(geom);

  // setup physics list
  auto physlist = ::js["phys_list"];
  auto chemlist = ::js["chem_list"];

  auto plist = PhysicsList::GetInstance();
  plist->SetPhysics(physlist);
  plist->SetChemistry(chemlist);
  run->SetUserInitialization(plist);

  // for primary removal
  primary_removal_ = ::js["primary_removal"];
  kill_elow_ = ::js["kill_energy"][0].get<double>() * keV;
  kill_eupp_ = ::js["kill_energy"][1].get<double>() * keV;

  // setup electron solvation model
  auto mname = ::js["ele_solvation_model"];
  ::set_solvation_model(mname);

  // setup output file name
  std::string fname = ::js["output_file"];
  std::string fname_bench = ::js["benchmark_file"];
  auto sd = SimData::GetInstance();
  sd->SetFileName(fname);
  sd->SetBenchmarkFileName(fname_bench);
  sd->SetThreadNumber(num_thread_);

}
