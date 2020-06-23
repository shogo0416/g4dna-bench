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
#include "primary_generator.h"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"

//------------------------------------------------------------------------------
PrimaryGenerator::PrimaryGenerator()
    : G4VUserPrimaryGeneratorAction()
{
  SetParticle("e-");
  SetEnergy(750.0 * keV);
  SetPosition(0.0, 0.0, 0.0);
  SetDirection(0.0, 0.0, 1.0);
}

//------------------------------------------------------------------------------
void PrimaryGenerator::GeneratePrimaries(G4Event* event)
{
  double mass = particle_->GetPDGMass();
  double p = sqrt(sqr(mass + ekin_) - sqr(mass));
  G4ThreeVector pm = p * dir_;
  auto primary = new G4PrimaryParticle(particle_, pm.x(), pm.y(), pm.z());
  auto vertex = new G4PrimaryVertex(pos_, 0.0);
  vertex->SetPrimary(primary);
  event->AddPrimaryVertex(vertex);
}
