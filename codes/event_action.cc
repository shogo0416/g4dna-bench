/*==============================================================================
  BSD 2-Clause License

  Copyright (c) 2020-2022 Shogo OKADA (shogo.okada@kek.jp)
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
#include "event_action.h"
#include "timehistory.h"
#include "simdata.h"
#include "G4Threading.hh"
#include "G4Event.hh"

namespace {

static auto timer = TimeHistory::GetTimeHistory();
static auto simdata = SimData::GetInstance();

} // end of anonymous namespace

//==============================================================================
EventAction::EventAction()
    : term_frequency_{1000}
{
}

//------------------------------------------------------------------------------
void EventAction::BeginOfEventAction(const G4Event*)
{
  if (!::simdata->ThreadBenchmarkTestIsEnabled()) { return; }
  start_time_ = ::timer->TakeSplit();
}

//------------------------------------------------------------------------------
void EventAction::EndOfEventAction(const G4Event* event)
{

  if (::simdata->ThreadBenchmarkTestIsEnabled()) {

    stop_time_ = ::timer->TakeSplit();

#ifdef G4MULTITHREADED
    int id = G4Threading::G4GetThreadId();
#else
    constexpr int id = 0;
#endif

    double elap_time = stop_time_ - start_time_;

    ::simdata->GetTotElapTime()[id] += elap_time;

  }

  int event_id = event->GetEventID();

  if (event_id % term_frequency_ == 0 && event_id != 0) {
    std::cout << "[MESSAGE] event-loop check point: "
              << event_id << " events processed." << std::endl;
  }

}
