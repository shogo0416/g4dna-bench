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
#include "molecule_counter.h"
#include "simdata.h"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"
#include "G4MoleculeCounter.hh"
#include "G4MolecularConfiguration.hh"
#include "G4MoleculeTable.hh"
#include "G4Threading.hh"

//------------------------------------------------------------------------------
MoleculeCounter::MoleculeCounter(G4String name, G4int depth)
    : G4VPrimitiveScorer(name, depth)

{
  simdata_ = SimData::GetInstance();
}

//------------------------------------------------------------------------------
G4bool MoleculeCounter::ProcessHits(G4Step* step, G4TouchableHistory*)
{
#ifdef G4MULTITHREADED
  int id = G4Threading::G4GetThreadId();
#else
  int id = 0;
#endif
  double edep = step->GetTotalEnergyDeposit();
  if (edep > 0) { simdata_->AccumulateEdep(id, edep); }
  return true;
}

//------------------------------------------------------------------------------
void MoleculeCounter::Initialize(G4HCofThisEvent*)
{
}

//------------------------------------------------------------------------------
void MoleculeCounter::EndOfEvent(G4HCofThisEvent*)
{
#ifdef G4MULTITHREADED
  int id = G4Threading::G4GetThreadId();
#else
  int id = 0;
#endif

  auto eman = G4EventManager::GetEventManager();
  if (eman->GetConstCurrentEvent()->IsAborted()) {
    clear();
    simdata_->CountAbortEvent(id);
    return;
  }

  auto counter = G4MoleculeCounter::Instance();
  auto species = counter->GetRecordedMolecules();
  if (species.get() == 0 || species->size() == 0) {
    clear();
    simdata_->CountAbortEvent(id);
    return;
  }

  static auto score_time = simdata_->GetScoreTime();
  double edep_factor = 100.0 / (simdata_->GetEdep(id) / eV);

  for (auto mol : *species) {
    std::string name = mol->GetName();
    int tid = 0;
    for (auto t : score_time) {
      int nmol = counter->GetNMoleculesAtTime(mol, t);
      double gval = nmol * edep_factor;
      simdata_->GValue(id, tid, name, gval);
      tid++;
    }
  }

  simdata_->CountChemEvent(id);

  clear();
}

//------------------------------------------------------------------------------
void MoleculeCounter::clear()
{
#ifdef G4MULTITHREADED
  int id = G4Threading::G4GetThreadId();
#else
  int id = 0;
#endif
  simdata_->ResetEdep(id);
  G4MoleculeCounter::Instance()->ResetCounter();
}

//------------------------------------------------------------------------------
void MoleculeCounter::DrawAll()
{
}

//------------------------------------------------------------------------------
void MoleculeCounter::PrintAll()
{
}
