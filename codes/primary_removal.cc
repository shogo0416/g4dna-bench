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
#include "primary_removal.h"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"

//------------------------------------------------------------------------------
PrimaryRemoval::PrimaryRemoval(G4String name, G4int depth)
    : G4VPrimitiveScorer(name, depth)
{
  elow_ = DBL_MAX;
  eupp_ = DBL_MAX;
  total_eloss_ = 0.0;
}

//------------------------------------------------------------------------------
G4bool PrimaryRemoval::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  auto track = step->GetTrack();
  if (track->GetTrackID() != 1) { return false; }

  // get current kinetic energy
  double ekin = step->GetPostStepPoint()->GetKineticEnergy();

  // calculate energy loss at the step
  double eloss = step->GetPreStepPoint()->GetKineticEnergy() - ekin;
  if (eloss <= 0.0) { return false; }

  // cumulative energy loss
  total_eloss_ += eloss;

  // stop event processing
  if (total_eloss_ >= eupp_) { G4RunManager::GetRunManager()->AbortEvent(); }

  // stop tracking
  if (total_eloss_ >= elow_) { track->SetTrackStatus(fStopAndKill); }

  return true;
}

//------------------------------------------------------------------------------
void PrimaryRemoval::Initialize(G4HCofThisEvent*)
{
  clear();
}

//------------------------------------------------------------------------------
void PrimaryRemoval::EndOfEvent(G4HCofThisEvent*)
{
}

//------------------------------------------------------------------------------
void PrimaryRemoval::clear()
{
  total_eloss_ = 0.0;
}

//------------------------------------------------------------------------------
void PrimaryRemoval::DrawAll()
{
}

//------------------------------------------------------------------------------
void PrimaryRemoval::PrintAll()
{
}
