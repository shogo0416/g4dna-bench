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
#include "G4Molecule.hh"

namespace {

//------------------------------------------------------------------------------
int find_lower_bound(const std::vector<TimeStepInfo>& info, double x)
{
  int low = 0;
  int upp = info.size();
  while (low <= upp) {
    int mid_bin = (low + upp) * 0.5;
    if (x < info[mid_bin].sim_time) { upp = mid_bin - 1; }
    else { low = mid_bin + 1; }
  }
  if (upp < 0) { upp = 0; }
  return upp;
}
//------------------------------------------------------------------------------
int interpolate(double t, double t1, double t2, int n1, int n2)
{
  int diff = n2 - n1;
  if (diff == 0) { return n1; }

  double a = (double)(diff) / (log10(t2) - log10(t1));
  double b = (double)(n2) - a * log10(t2);
  double val = round(a * log10(t) + b);

  return (int)(val);
}

} // end of anonymous namespace

//==============================================================================

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
  constexpr int id = 0;
#endif

  double edep = step->GetTotalEnergyDeposit();
  if (edep > 0.0) { simdata_->AccumulateEdep(id, edep); }

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
  constexpr int id = 0;
#endif

  auto eman = G4EventManager::GetEventManager();
  if (eman->GetConstCurrentEvent()->IsAborted()) {
    clear();
    simdata_->CountAbortEvent(id);
    return;
  }

  static auto score_time = simdata_->GetScoreTime();
  double edep_factor = 100.0 / (simdata_->GetEdep(id) / eV);

  if (G4MoleculeCounter::InUse()) {

    auto counter = G4MoleculeCounter::Instance();
    auto species = counter->GetRecordedMolecules();
    if (species.get() == 0 || species->size() == 0) {
      clear();
      simdata_->CountAbortEvent(id);
      return;
    }

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

  } else {

    auto tsi = simdata_->GetTimeStepInfo(id);
    if (tsi.size() == 0) {
      clear();
      simdata_->CountAbortEvent(id);
      return;
    }

    int tid = 0;
    for (auto t : score_time) {

      int id1 = ::find_lower_bound(tsi, t);
      int id2 = id1 + 1;
      double t1 = tsi[id1].sim_time;
      double t2 = tsi[id2].sim_time;

      for (auto x : simdata_->GetScoredMolecule()) {
        auto name = x.first;
        int n1 = tsi[id1].species[name];
        int n2 = tsi[id2].species[name];
        int n  = ::interpolate(t, t1, t2, n1, n2);

        double gval = n * edep_factor;
        simdata_->GValue(id, tid, name, gval);

      }

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
  constexpr int id = 0;
#endif
  simdata_->ResetEdep(id);
  if (G4MoleculeCounter::InUse()) {
    G4MoleculeCounter::Instance()->ResetCounter();
  } else {
    simdata_->ClearTimeStepInfo(id);
  }
}

//------------------------------------------------------------------------------
void MoleculeCounter::DrawAll()
{
}

//------------------------------------------------------------------------------
void MoleculeCounter::PrintAll()
{
}
