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
#include "time_step_action.h"
#include "geometry.h"
#include "simdata.h"
#include "G4Scheduler.hh"
#include "G4Threading.hh"
#include "G4SystemOfUnits.hh"
#include "G4Scheduler.hh"
#include "G4ITTrackHolder.hh"
#include "G4Molecule.hh"
#include "G4MoleculeCounter.hh"
#include "G4Step.hh"
#include "G4Version.hh"
#include "G4MoleculeTable.hh"
#include "G4MolecularConfiguration.hh"

//------------------------------------------------------------------------------
TimeStepAction::TimeStepAction()
    : G4UserTimeStepAction()
{
  check_boundary_ = false;
}

//------------------------------------------------------------------------------
void TimeStepAction::UserPreTimeStepAction()
{
#ifdef G4MULTITHREADED
  int id = G4Threading::G4GetThreadId();
#else
  constexpr int id = 0;
#endif

  const double time = G4Scheduler::Instance()->GetGlobalTime();
  if (time > 1.0 * picosecond) { return; }

  Reset();

  auto track_holder = G4ITTrackHolder::Instance();
  auto list = track_holder->GetMainList();

  for (auto x : *list) { Count(x); }

  TimeStepInfo tsi = {time, mcounter_};
  SimData::GetInstance()->PushTimeStepInfo(id, tsi, true);
}

//------------------------------------------------------------------------------
void TimeStepAction::UserPostTimeStepAction()
{

#if G4VERSION_NUMBER >= 1140
  auto* mcman = G4MoleculeCounterManager::Instance();
  const auto inuse = mcman->GetIsActive();
#elif G4VERSION_NUMBER >= 1110
  const auto inuse = G4MoleculeCounter::Instance()->InUse();
#else
  const auto inuse = G4MoleculeCounter::InUse();
#endif
  if (inuse) { return; }

#ifdef G4MULTITHREADED
  int id = G4Threading::G4GetThreadId();
#else
  constexpr int id = 0;
#endif

  Reset();

  auto track_holder = G4ITTrackHolder::Instance();
  auto list = track_holder->GetMainList();

  for (auto x : *list) { Count(x); }

  const double time = G4Scheduler::Instance()->GetGlobalTime();

  TimeStepInfo tsi = {time, mcounter_};
  SimData::GetInstance()->PushTimeStepInfo(id, tsi);
}

//------------------------------------------------------------------------------
void TimeStepAction::EndProcessing()
{

#ifdef G4MULTITHREADED
  int id = G4Threading::G4GetThreadId();
#else
  constexpr int id = 0;
#endif

  auto simdata = SimData::GetInstance();
  simdata->GetNumChemStep()[id] += G4Scheduler::Instance()->GetNbSteps();

}

//------------------------------------------------------------------------------
void TimeStepAction::Reset()
{
  mcounter_.clear();
}

//------------------------------------------------------------------------------
void TimeStepAction::Count(G4Track* trk)
{
  if (!CheckInVolume(trk)) {
    trk->SetTrackStatus(fStopAndKill);
    return;
  }

  auto check_molecule = [](const G4MolecularConfiguration* mconf) {
    static auto H3OpB = G4MoleculeTable::Instance()->GetConfiguration("H3Op(B)");
    static auto OHmB  = G4MoleculeTable::Instance()->GetConfiguration("OHm(B)");
    if (mconf == H3OpB || mconf == OHmB) { return true; }
    return false;
  };

  const auto mconf = GetMolecule(trk)->GetMolecularConfiguration();
  if (check_molecule(mconf)) { return; }

  const auto name = mconf->GetName();
  auto x = mcounter_.find(name);
  if (x != mcounter_.end()) {
    mcounter_[name] += 1;
  } else {
    mcounter_[name] = 1;
  }
}

//------------------------------------------------------------------------------
void TimeStepAction::CheckBoundary(bool in)
{
  check_boundary_ = in;

  if (!check_boundary_) { return; }

  auto box_size = Geometry::GetInstance()->GetPhantomSize();

  upp_bound_x_ = box_size.x() * 0.5;
  upp_bound_y_ = box_size.y() * 0.5;
  upp_bound_z_ = box_size.z() * 0.5;
  low_bound_x_ = -1.0 * upp_bound_x_;
  low_bound_y_ = -1.0 * upp_bound_y_;
  low_bound_z_ = -1.0 * upp_bound_z_;
}

//------------------------------------------------------------------------------
bool TimeStepAction::CheckInVolume(G4Track* trk)
{
  if (!check_boundary_) { return true; }

  auto pos = trk->GetPosition();

  if (pos.x() < low_bound_x_ || pos.x() >= upp_bound_x_) { return false; }
  if (pos.y() < low_bound_y_ || pos.y() >= upp_bound_y_) { return false; }
  if (pos.z() < low_bound_z_ || pos.z() >= upp_bound_z_) { return false; }

  return true;
}
