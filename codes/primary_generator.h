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
#ifndef PRIMARY_GENERATOR_H_
#define PRIMARY_GENERATOR_H_
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ThreeVector.hh"

class G4PrimaryVertex;
class G4Event;

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGenerator();
  virtual ~PrimaryGenerator() = default;

  virtual void GeneratePrimaries(G4Event* event);

  // set/get methods
  void SetParticle(std::string pkind, int Z = 0, int A = 0);
  std::string GetParticle() const;

  void SetEnergy(double ekin);
  double GetEnergy() const;

  void SetPosition(double x, double y, double z);
  G4ThreeVector GetPosition();

  void SetDirection(double x, double y, double z);
  G4ThreeVector GetDirection();

private:
  G4ThreeVector pos_;
  G4ThreeVector dir_;
  double ekin_;
  std::string pkind_;
  G4ParticleDefinition* particle_;
};

//==============================================================================
inline void PrimaryGenerator::SetPosition(double x, double y, double z)
{
  pos_.set(x, y, z);
}

//------------------------------------------------------------------------------
inline G4ThreeVector PrimaryGenerator::GetPosition()
{
  return pos_;
}

//------------------------------------------------------------------------------
inline void PrimaryGenerator::SetDirection(double x, double y, double z)
{
  dir_.set(x, y, z);
}

//------------------------------------------------------------------------------
inline G4ThreeVector PrimaryGenerator::GetDirection()
{
  return dir_;
}

//------------------------------------------------------------------------------
inline void PrimaryGenerator::SetEnergy(double ekin)
{
  ekin_ = ekin;
}

//------------------------------------------------------------------------------
inline double PrimaryGenerator::GetEnergy() const
{
  return ekin_;
}

//------------------------------------------------------------------------------
inline std::string PrimaryGenerator::GetParticle() const
{
  return pkind_;
}

#endif // PRIMARY_GENERATOR_H_
