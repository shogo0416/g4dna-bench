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
#include "G4Version.hh"
#if G4VERSION_NUMBER >= 1100
#include "G4LowEnergyEmProcessSubType.hh"
#endif

namespace {

constexpr bool kPreStep = true;
constexpr int  kElectronPDGID = 11;

#if G4VERSION_NUMBER >= 1100
constexpr int kChargeDecrease = fLowEnergyChargeDecrease;
constexpr int kChargeIncrease = fLowEnergyChargeIncrease;
#else
constexpr int kChargeDecrease = 56;
constexpr int kChargeIncrease = 57;
#endif

//------------------------------------------------------------------------------
int find_lower_bound(const std::vector<TimeStepInfo>& info, double x)
{
  auto low = 0;
  auto upp = info.size();
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
  auto diff = n2 - n1;
  if (diff == 0) { return n1; }

  auto a = static_cast<double>(diff) / (log10(t2) - log10(t1));
  auto b = static_cast<double>(n2) - a * log10(t2);
  auto val = round(a * log10(t) + b);

  return static_cast<int>(val);
}

} // end of anonymous namespace

//==============================================================================

MoleculeCounter::MoleculeCounter(G4String name, G4int depth)
    : G4VPrimitiveScorer(name, depth),
      edep_{0.0},
      accum_steplen_{0.0},
      accum_ekin_{0.0}
{
  simdata_ = SimData::GetInstance();
}

//------------------------------------------------------------------------------
G4bool MoleculeCounter::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  double edep = step->GetTotalEnergyDeposit();
  if (edep > 0.0) { edep_ += edep; }

  const auto track = step->GetTrack();

  const bool is_primary = (step->GetTrack()->GetTrackID() == 1);

  // stop process for secondary particles
  if (!is_primary) {
    // skip electrons
    if (track->GetParticleDefinition()->GetPDGEncoding() == ::kElectronPDGID) {
      return false;
    }
    // skip particles generated through processes other than
    // "charge change" processes
    const auto sub_type = track->GetCreatorProcess()->GetProcessSubType();
    if (sub_type != ::kChargeDecrease && sub_type != ::kChargeIncrease) {
      return false;
    }
  }

  accum_ekin_ += edep;
  accum_steplen_ += step->GetStepLength();

  const auto sub_type
    = step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessSubType();

  // skip "charge change" processes
  if (sub_type == ::kChargeDecrease || sub_type == ::kChargeIncrease) {
    return true;
  }

  const auto secondary = step->GetSecondaryInCurrentStep();
  const int num = (*secondary).size();
  for (int i = 0; i < num; i++) {
    accum_ekin_ += (*secondary)[i]->GetKineticEnergy();
  }

  return true;
}

//------------------------------------------------------------------------------
void MoleculeCounter::Initialize(G4HCofThisEvent*)
{
  clear();
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

  // ===========================================================================
  //  Store G-values for each molecular species
  // ===========================================================================

  auto check_molecule = [](const G4MolecularConfiguration* mconf) {
    static auto H3OpB = G4MoleculeTable::Instance()->GetConfiguration("H3Op(B)");
    static auto OHmB  = G4MoleculeTable::Instance()->GetConfiguration("OHm(B)");
    if (mconf == H3OpB || mconf == OHmB) { return true; }
    return false;
  };

  static auto score_time = simdata_->GetScoreTime();
  double edep_factor = 100.0 / (edep_ / eV);

#if G4VERSION_NUMBER >= 1140
  // ver 11.4 ~
  auto* mcman = G4MoleculeCounterManager::Instance();
  auto inuse  = mcman->GetIsActive();
#else
  // ~ ver 11.3
  auto* counter = G4MoleculeCounter::Instance();
#if G4VERSION_NUMBER >= 1110
  auto inuse = counter->InUse();
#else
  auto inuse = G4MoleculeCounter::InUse();
#endif // G4VERSION_NUMBER >= 1110
#endif // G4VERSION_NUMBER >= 1140

  if (inuse) {

#if G4VERSION_NUMBER >= 1140

    auto counter = mcman->GetMoleculeCounter<G4MoleculeCounter>(0);
    auto indices = counter->GetMapIndices();
    if (indices.empty()) {
      clear();
      simdata_->CountAbortEvent(id);
      return;
    }

    for (auto idx : indices) {

    }

#else
    auto species = counter->GetRecordedMolecules();
    if (species.get() == 0 || species->size() == 0) {
      clear();
      simdata_->CountAbortEvent(id);
      return;
    }

    for (auto mol : *species) {
      if (check_molecule(mol)) { continue; }
      const std::string name = mol->GetName();
      int tid = 0;
      for (auto t : score_time) {
        int nmol = counter->GetNMoleculesAtTime(mol, t);
        double gval = nmol * edep_factor;
        simdata_->GValue(id, tid, name, gval);
        tid++;
      }
    }

#endif // G4VERSION_NUMBER >= 1140

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

  // ===========================================================================
  //  Store energy deposit and LET
  // ===========================================================================
  auto LET = (accum_ekin_ / keV) / (accum_steplen_ / um);
  simdata_->PushLETInfo(id, std::make_pair(edep_ / eV, LET));

  // number of species generated at 1 ps vs process time for the chemistry stage
  auto tsi = simdata_->GetTimeStepInfo(id, ::kPreStep);
  auto etc = simdata_->GetElapTimeChem()[id];
  ChemInfo ci = {etc, tsi[0].species};
  simdata_->PushChemInfo(id, ci);

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

  edep_ = 0.0;
  accum_steplen_ = 0.0;
  accum_ekin_ = 0.0;

#if G4VERSION_NUMBER >= 1140

  auto* mcman = G4MoleculeCounterManager::Instance();
  if (!mcman->GetIsActive()) {
    simdata_->ClearTimeStepInfo(id);
  }

#else

  auto* counter = G4MoleculeCounter::Instance();
#if G4VERSION_NUMBER >= 1110
  auto inuse = counter->InUse();
#else
  auto inuse = G4MoleculeCounter::InUse();
#endif

  if (inuse) {
    counter->ResetCounter();
  } else {
    simdata_->ClearTimeStepInfo(id);
  }

#endif // G4VERSION_NUMBER >= 1140

  simdata_->ClearTimeStepInfo(id, ::kPreStep);
}

//------------------------------------------------------------------------------
void MoleculeCounter::DrawAll()
{
}

//------------------------------------------------------------------------------
void MoleculeCounter::PrintAll()
{
}
