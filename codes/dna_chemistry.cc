/*==============================================================================
  BSD 2-Clause License

  Copyright (c) 2020-2025 Shogo OKADA (shogo.okada@kek.jp)
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
#include "dna_chemistry.h"
#include "dna_dissociation_channel.h"

#include "G4DNAElectronSolvation.hh"
#include "G4DNAVibExcitation.hh"
#include "G4DNASancheExcitationModel.hh"

#include "G4Electron.hh"
#include "G4MoleculeTable.hh"
//#include "G4Molecule.hh"
#include "G4H2O.hh"
//#include "G4FakeMolecule.hh"

#include "G4DNAWaterDissociationDisplacer.hh"
#include "G4DNABrownianTransportation.hh"
#include "G4DNAElectronHoleRecombination.hh"
#include "G4DNAMolecularDissociation.hh"

#include "G4PhysicsConstructorFactory.hh"
#include "G4PhysicsListHelper.hh"
#include "G4ProcessTable.hh"

namespace {

void ConstructProcess()
{
  auto* ph = G4PhysicsListHelper::GetPhysicsListHelper();

  //===============================================================
  // Extend vibrational to low energy
  // Anyway, solvation of electrons is taken into account from 7.4 eV
  // So below this threshold, for now, no accurate modeling is done
  //
  G4VProcess* process{nullptr};

  auto* ptab = G4ProcessTable::GetProcessTable();

  process = ptab->FindProcess("e-_G4DNAVibExcitation", "e-");

  if (process) {
    auto* vibexc = static_cast<G4DNAVibExcitation*>(process);
    auto* sanche = static_cast<G4DNASancheExcitationModel*>(vibexc->EmModel());
    if (sanche) { sanche->ExtendLowEnergyLimit(0.025 * eV); }
  }

  //===============================================================
  // *** Electron Solvatation ***
  //
  process = ptab->FindProcess("e-_G4DNAElectronSolvation", "e-");

  if (!process) {
    ph->RegisterProcess(new G4DNAElectronSolvation("e-_G4DNAElectronSolvation"),
                        G4Electron::Definition());
  }

  //===============================================================
  // Define processes for molecules
  //
  auto* mtab = G4MoleculeTable::Instance();

  G4MoleculeDefinitionIterator iterator = mtab->GetDefintionIterator();
  iterator.reset();

  while (iterator()) {
    G4MoleculeDefinition* mdef = iterator.value();

    if (mdef != G4H2O::Definition()) {

      //if (model_option_ == SBS) {
      //  ph->RegisterProcess(new G4DNABrownianTransportation(), mdef);
      //}

    } else {

      // electron hole recombination process
      auto* pm = mdef->GetProcessManager();
      pm->AddRestProcess(new G4DNAElectronHoleRecombination(), 2);

      // dissociation processes for ionized- and excited water molecules
      // at the physico-chemical stage
      auto* decay = new G4DNAMolecularDissociation("H2O_DNAMolecularDecay");
      decay->SetDisplacer(mdef,
                          new G4DNAWaterDissociationDisplacer());
      decay->SetVerboseLevel(1);
      pm->AddRestProcess(decay, 1);

    }
    /*
     * Warning : end of particles and processes are needed by
     * EM Physics builders
     */
  }

  G4DNAChemistryManager::Instance()->Initialize();
}

} // end of anonymous namespace
//==============================================================================

G4_DECLARE_PHYSCONSTR_FACTORY(DNAChemistryOpt3);

//------------------------------------------------------------------------------
void DNAChemistryOpt3::ConstructDissociationChannels()
{
#if G4VERSION_NUMBER >= 1130
  DNADissociationChannel::ConstructDissociationChannels(false, false);
#endif
}

//------------------------------------------------------------------------------
void DNAChemistryOpt3::ConstructProcess()
{
  ::ConstructProcess();
}
