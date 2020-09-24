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
#ifndef TRACKING_INTERACTIVITY_H_
#define TRACKING_INTERACTIVITY_H_
#include "G4ITTrackingInteractivity.hh"

class G4Track;
class G4Step;
class G4UserSteppingAction;

class TrackingInteractivity: public G4ITTrackingInteractivity {
public:
  TrackingInteractivity();
  ~TrackingInteractivity() = default;
  void AppendStep(G4Track*, G4Step* step);
  void SetSteppingAction(G4UserSteppingAction* ptr);

private:
  G4UserSteppingAction* step_action_;
};

//==============================================================================
inline void TrackingInteractivity::SetSteppingAction(
  G4UserSteppingAction* ptr)
{
  step_action_ = ptr;
}

#endif // TRACKING_INTERACTIVITY_H_
