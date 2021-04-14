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
#include "geometry.h"
#include "application.h"
#include "primary_removal.h"
#include "molecule_counter.h"
#include "G4MultiFunctionalDetector.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"

Geometry* Geometry::instance_ = nullptr;

//------------------------------------------------------------------------------
Geometry::Geometry()
    : G4VUserDetectorConstruction()
{
  debug_ = false;
}

//------------------------------------------------------------------------------
Geometry* Geometry::GetInstance()
{
  if (!instance_) { instance_ = new Geometry(); }
  return instance_;
}

//------------------------------------------------------------------------------
G4VPhysicalVolume* Geometry::Construct()
{

  constexpr bool check_overlap = true;

  // set up material
  auto water = G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER");

  // set up phantom volume
  auto psol = new G4Box("Phantom",
                        psize_.x() * 0.5,
                        psize_.y() * 0.5,
                        psize_.z() * 0.5);

  // make a logical volume
  auto plv = new G4LogicalVolume(psol, water, "Phantom");

  // make a physical volume
  ppv_ = new G4PVPlacement(0,               // no rotation
                           G4ThreeVector(), // at (0,0,0)
                           plv,             // its logical volume
                           "Phantom",       // its name
                           nullptr,         // its mother volume
                           false,           // no boolean operator
                           0,               // copy number
                           check_overlap);  // overlaps checking

  // visualization attributes
  auto vis_att = new G4VisAttributes(G4Color(1, 1, 1));
  plv->SetVisAttributes(vis_att);

  // print geometry info
  if (debug_ ) { Print(); }

  return ppv_;
}

//------------------------------------------------------------------------------
void Geometry::ConstructSDandField()
{

  auto mfd = new G4MultiFunctionalDetector("SD");

  auto app = Application::GetInstance();

  if (app->PrimaryRemovalIsEnabled()) {

    double elow = app->GetKillEnergyLowLim();
    double eupp = app->GetKillEnergyUppLim();

    auto pr = new PrimaryRemoval("PrimaryRemoval");
    pr->SetEnergyThreshold(elow, eupp);
    mfd->RegisterPrimitive(pr);

    if (debug_) {

#ifdef G4MULTITHREADED
      int id = G4Threading::G4GetThreadId();
#else
      constexpr int id = 0;
#endif

      std::cout << "[Geometry::MESSAGE] Set PrimaryRemoval (Thread#:" << id
                << ", Elow: " << elow / keV << " keV, Eupp: " << eupp / keV
                << " keV)" << std::endl;
    }
  }

  mfd->RegisterPrimitive(new MoleculeCounter("MoleculeCounter"));

  G4SDManager::GetSDMpointer()->AddNewDetector(mfd);
  SetSensitiveDetector("Phantom", mfd);
}

//------------------------------------------------------------------------------
void Geometry::Print()
{
  std::cout << "[Geometry::MESSAGE] Phantom Volume: " << std::endl;
  std::cout << "  - Size: "
            << psize_[0] / um << " x "
            << psize_[1] / um << " x "
            << psize_[2] / um << " um" << std::endl;
  std::cout << "  - Material: "
            << ppv_->GetLogicalVolume()->GetMaterial()->GetName()
            << std::endl;
}
