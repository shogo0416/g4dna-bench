/*==============================================================================
  BSD 2-Clause License

  Copyright (c) 2020-2025 Shogo OKADA (shogo.okada@kek.jp)
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
#ifndef PHYSICS_LIST_H_
#define PHYSICS_LIST_H_
#include "globals.hh"
#include "G4VModularPhysicsList.hh"
#include "G4VUserChemistryList.hh"
#include "G4Version.hh"
#include "dna_chemistry.h"
#include <string>

class G4VPhysicsConstructor;

class PhysicsList: public G4VModularPhysicsList {
public:
  static PhysicsList* GetInstance();
  ~PhysicsList() = default;

  PhysicsList(const PhysicsList&) = delete;
  void operator=(PhysicsList&) = delete;

  void SetPhysics(const std::string& name);
  void SetChemistry(G4VPhysicsConstructor* in);
  void SetChemistry(const std::string& name);

#if G4VERSION_NUMBER >= 1130
  void SetTimeStepModel(const std::string& name);
  void EnableMultipleIonisation(bool in);
  DNABaseChemistry* GetBaseChemistry(const std::string& name);
#endif

  G4VPhysicsConstructor* GetPhysics();
  G4VPhysicsConstructor* GetChemistry();

  void ConstructProcess();
  void ConstructParticle();

private:
  PhysicsList();
  static PhysicsList* instance_;

  G4VPhysicsConstructor* phys_list_;
  G4VPhysicsConstructor* chem_list_;

#if G4VERSION_NUMBER >= 1130
  void SetAltChemistry(const std::string& name);
  void ConstructMultipleIonisationProcess();
  bool enable_mioni_;
  DNABaseChemistry* base_chem_;
#endif
};

//==============================================================================
inline G4VPhysicsConstructor* PhysicsList::GetPhysics()
{
  return phys_list_;
}

//------------------------------------------------------------------------------
inline G4VPhysicsConstructor* PhysicsList::GetChemistry()
{
  return chem_list_;
}

//------------------------------------------------------------------------------
#if G4VERSION_NUMBER >= 1130
inline void PhysicsList::EnableMultipleIonisation(bool in)
{
  enable_mioni_ = in;
}
#endif

//------------------------------------------------------------------------------
inline void PhysicsList::SetChemistry(G4VPhysicsConstructor* in)
{
  chem_list_ = in;
}

//------------------------------------------------------------------------------
inline DNABaseChemistry* PhysicsList::GetBaseChemistry(const std::string& name)
{
  if (!enable_mioni_) { return nullptr; }

  if (name == "G4EmDNAChemistry") {
    auto* ptr = static_cast<DNAChemistry*>(GetChemistry());
    return static_cast<DNABaseChemistry*>(ptr);
  } else if (name == "G4EmDNAChemistry_option1") {
    auto* ptr = static_cast<DNAChemistryOpt1*>(GetChemistry());
    return static_cast<DNABaseChemistry*>(ptr);
  } else if (name == "G4EmDNAChemistry_option2") {
    auto* ptr = static_cast<DNAChemistryOpt2*>(GetChemistry());
    return static_cast<DNABaseChemistry*>(ptr);
  } else if (name == "G4EmDNAChemistry_option3") {
    auto* ptr = static_cast<DNAChemistryOpt3*>(GetChemistry());
    return static_cast<DNABaseChemistry*>(ptr);
  }

  return nullptr;
}

#endif // PHYSICS_LIST_H_
