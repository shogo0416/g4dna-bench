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
#include "physics_list.h"

#include "G4Version.hh"
#include "G4PhysicsConstructorRegistry.hh"
#include "G4SystemOfUnits.hh"
#include "G4EmDNAPhysics.hh"
#include "G4EmDNAPhysics_option1.hh"
#include "G4EmDNAPhysics_option2.hh"
#include "G4EmDNAPhysics_option3.hh"
#include "G4EmDNAPhysics_option4.hh"
#include "G4EmDNAPhysics_option5.hh"
#include "G4EmDNAPhysics_option6.hh"
#include "G4EmDNAPhysics_option7.hh"
#include "G4EmDNAPhysics_option8.hh"
#include "G4EmDNAChemistry.hh"
#include "G4EmDNAChemistry_option1.hh"

#if G4VERSION_NUMBER >= 1060

#include "G4EmDNAChemistry_option2.hh"
#include "G4EmDNAChemistry_option3.hh"

#else

#include "G4ProcessTable.hh"
#include "G4VProcess.hh"
#include "G4PhysicsListHelper.hh"
#include "G4Electron.hh"
#include "G4DNAVibExcitation.hh"
#include "G4DNAAttachment.hh"

namespace {

void add_physics_process()
{
  auto ptab = G4ProcessTable::GetProcessTable();
  auto ph = G4PhysicsListHelper::GetPhysicsListHelper();
  auto ele = G4Electron::Electron();

  // check vibrational excitation process
  G4VProcess* proc = ptab->FindProcess("e-_G4DNAVibExcitation", "e-");
  if (!proc) {
    ph->RegisterProcess(new G4DNAVibExcitation("e-_G4DNAVibExcitation"), ele);
  }

  // check dissociative attachment process
  proc = ptab->FindProcess("e-_G4DNAAttachment", "e-");
  if (!proc) {
    ph->RegisterProcess(new G4DNAAttachment("e-_G4DNAAttachment"), ele);
  }
}

} // end of anonymous namespace

#endif // G4VERSION_NUMBER >= 1060

//==============================================================================

PhysicsList* PhysicsList::instance_ = nullptr;

//------------------------------------------------------------------------------
PhysicsList::PhysicsList()
    : G4VModularPhysicsList(),
      phys_list_(nullptr),
      chem_list_(nullptr)
{
  auto* ptab = G4ProductionCutsTable::GetProductionCutsTable();
  ptab->SetEnergyRange(100.0 * eV, 1.0 * GeV);

  SetDefaultCutValue(1.0 * nm);
  SetVerboseLevel(1);

  SetPhysics("G4EmDNAPhysics");
  SetChemistry("G4EmDNAChemistry");
}

//------------------------------------------------------------------------------
PhysicsList* PhysicsList::GetInstance()
{
  if (!instance_) { instance_ = new PhysicsList(); }
  return instance_;
}

//------------------------------------------------------------------------------
void PhysicsList::SetPhysics(std::string name)
{
  if (name == "G4EmDNAPhysics") {
    phys_list_ = new G4EmDNAPhysics();
  } else if (name == "G4EmDNAPhysics_option1") {
    phys_list_ = new G4EmDNAPhysics_option1();
  } else if (name == "G4EmDNAPhysics_option2") {
    phys_list_ = new G4EmDNAPhysics_option2();
  } else if (name == "G4EmDNAPhysics_option3") {
    phys_list_ = new G4EmDNAPhysics_option3();
  } else if (name == "G4EmDNAPhysics_option4") {
    phys_list_ = new G4EmDNAPhysics_option4();
  } else if (name == "G4EmDNAPhysics_option5") {
    phys_list_ = new G4EmDNAPhysics_option5();
  } else if (name == "G4EmDNAPhysics_option6") {
    phys_list_ = new G4EmDNAPhysics_option6();
  } else if (name == "G4EmDNAPhysics_option7") {
    phys_list_ = new G4EmDNAPhysics_option7();
  } else if (name == "G4EmDNAPhysics_option8") {
    phys_list_ = new G4EmDNAPhysics_option8();
  } else {
    std::cerr << "[ERROR] Set unknown list (name: " << name << ")" <<std::endl;
    std::exit(EXIT_FAILURE);
  }
}

//------------------------------------------------------------------------------
void PhysicsList::SetChemistry(std::string name)
{
  if (name == "G4EmDNAChemistry")
  {
    chem_list_ = new G4EmDNAChemistry();
  }
  else if (name == "G4EmDNAChemistry_option1")
  {
    chem_list_ = new G4EmDNAChemistry_option1();
  }
#if G4VERSION_NUMBER >= 1060
  else if (name == "G4EmDNAChemistry_option2")
  {
    chem_list_ = new G4EmDNAChemistry_option2();
  }
  else if (name == "G4EmDNAChemistry_option3")
  {
    chem_list_ = new G4EmDNAChemistry_option3();
  }
#endif
  else
  {
    std::cerr << "[ERROR] Set unknown list (name: " << name << ")" <<std::endl;
    std::exit(EXIT_FAILURE);
  }
}

//------------------------------------------------------------------------------
void PhysicsList::ConstructProcess()
{
  AddTransportation();
  phys_list_->ConstructProcess();
#if G4VERSION_NUMBER < 1060
  ::add_physics_process();
#endif
  chem_list_->ConstructProcess();
}

//------------------------------------------------------------------------------
void PhysicsList::ConstructParticle()
{
  phys_list_->ConstructParticle();
  chem_list_->ConstructParticle();
}
