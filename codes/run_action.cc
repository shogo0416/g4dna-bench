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
#include "run_action.h"
#include "simdata.h"
#include "timehistory.h"
#include "G4Run.hh"
#include "G4Threading.hh"
#include "G4Version.hh"

//------------------------------------------------------------------------------
void RunAction::BeginOfRunAction(const G4Run*)
{
  if (IsMaster()) {
    TimeHistory::GetTimeHistory()->TakeSplit("RunOn");
  }
}

//------------------------------------------------------------------------------
void RunAction::EndOfRunAction(const G4Run*)
{

  if (IsMaster()) {
    TimeHistory::GetTimeHistory()->TakeSplit("RunEnd");
    return;
  }

#ifdef G4MULTITHREADED

/*
#if G4VERSION_NUMBER >= 1020
  const bool is_master = G4Threading::IsMasterThread();
#else
  const bool is_master = G4Threading::IsWorkerThread();
#endif

  if (is_master) { return; }
*/
  int id = G4Threading::G4GetThreadId();
  SimData::GetInstance()->SaveSimulationResult(id);

#else
  constexpr int id = 0;
#endif

  SimData::GetInstance()->Performance(id);

}
