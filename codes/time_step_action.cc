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
#include "time_step_action.h"
#include "simdata.h"
#include "G4Scheduler.hh"
#include "G4Threading.hh"
#include "G4SystemOfUnits.hh"
#include "G4Scheduler.hh"
#include "G4ITTrackHolder.hh"
#include "G4Molecule.hh"
#include "G4MoleculeCounter.hh"

//------------------------------------------------------------------------------
TimeStepAction::TimeStepAction()
    : G4UserTimeStepAction()
{
}

//------------------------------------------------------------------------------
void TimeStepAction::UserPostTimeStepAction()
{

  if (G4MoleculeCounter::InUse()) { return; }

#ifdef G4MULTITHREADED
  int id = G4Threading::G4GetThreadId();
#else
  constexpr int id = 0;
#endif

  auto scheduler = G4Scheduler::Instance();
  const double time = scheduler->GetGlobalTime();

  Reset();

  auto track_holder = G4ITTrackHolder::Instance();
  auto list = track_holder->GetMainList();

  for (auto x : *list) {
    Count(x);
  }


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
  std::string name = GetMolecule(trk)->GetName();
  auto x = mcounter_.find(name);
  if (x != mcounter_.end()) {
    mcounter_[name] += 1;
  } else {
    mcounter_[name] = 1;
  }
}