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
#ifndef GEOMETRY_H_
#define GEOMETRY_H_
#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include <string>

class Geometry : public G4VUserDetectorConstruction {
public:
  static Geometry* GetInstance();
  virtual ~Geometry() = default;

  Geometry(const Geometry&) = delete;
  void operator=(const Geometry&) = delete;

  virtual G4VPhysicalVolume* Construct();
  virtual void ConstructSDandField();

  void SetPhantomSize(double x, double y, double z);
  G4ThreeVector GetPhantomSize();

  void DebugMode(bool in);

private:
  Geometry();
  static Geometry* instance_;

  bool debug_;
  G4ThreeVector psize_;
  G4VPhysicalVolume* ppv_;

  void Print();
};

//==============================================================================
inline void Geometry::SetPhantomSize(double x, double y, double z)
{
  psize_.set(x, y, z);
}

//------------------------------------------------------------------------------
inline G4ThreeVector Geometry::GetPhantomSize()
{
  return psize_;
}

//------------------------------------------------------------------------------
inline void Geometry::DebugMode(bool in)
{
  debug_ = in;
}

#endif // GEOMETRY_H_
