/*==============================================================================
  BSD 2-Clause License

  Copyright (c) 2020-2021 Shogo OKADA (shogo.okada@kek.jp)
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
#include "stacking_action.h"
#include "simdata.h"
#include "timehistory.h"
#include "G4StackManager.hh"
#include "G4DNAChemistryManager.hh"
#include "G4Threading.hh"

namespace {

static auto timer = TimeHistory::GetTimeHistory();
static auto simdata = SimData::GetInstance();

} // end of anonymous namespace

//------------------------------------------------------------------------------
StackingAction::StackingAction()
    : G4UserStackingAction()
{
}

//------------------------------------------------------------------------------
void StackingAction::NewStage()
{
  if (stackManager->GetNTotalTrack() != 0) { return; }

#ifdef G4MULTITHREADED
  int id = G4Threading::G4GetThreadId();
#else
  constexpr int id = 0;
#endif

  double start_time{0.0}, stop_time{0.0};

  bool benchmark_threads = ::simdata->ThreadBenchmarkTestIsEnabled();

  if (benchmark_threads) { start_time = ::timer->TakeSplit(); }

  // run chemical stage
  G4DNAChemistryManager::Instance()->Run();

  if (!benchmark_threads) { return; }

  stop_time = ::timer->TakeSplit();

  double elap_time = stop_time - start_time;
  ::simdata->GetTotElapTimeChem()[id] += elap_time;
  ::simdata->GetElapTimeChem()[id] = elap_time;
}
