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
#ifndef DNA_CHEMISTRY_H_
#define DNA_CHEMISTRY_H_

#include "G4EmDNAChemistry.hh"
#include "G4EmDNAChemistry_option1.hh"
#include "G4EmDNAChemistry_option2.hh"
#include "G4EmDNAChemistry_option3.hh"

//==============================================================================
class DNABaseChemistry {
public:
  DNABaseChemistry()
    : use_alt_B1A1_decay_{false},
      use_alt_decay_vibH2O_{false} {}
  ~DNABaseChemistry() = default;
  void UseAltB1A1Decay(bool in);
  void UseAltDecayVibH2O(bool in);
protected:
  bool use_alt_B1A1_decay_;
  bool use_alt_decay_vibH2O_;
};

//------------------------------------------------------------------------------
inline void DNABaseChemistry::UseAltB1A1Decay(bool in)
{
  use_alt_B1A1_decay_ = in;
}

//------------------------------------------------------------------------------
inline void DNABaseChemistry::UseAltDecayVibH2O(bool in)
{
  use_alt_decay_vibH2O_ = in;
}

//==============================================================================
class DNAChemistry : public DNABaseChemistry,
                     public G4EmDNAChemistry {
public:
  using G4EmDNAChemistry::G4EmDNAChemistry;
  void ConstructDissociationChannels() override;
};

//==============================================================================
class DNAChemistryOpt1 : public DNABaseChemistry,
                         public G4EmDNAChemistry_option1 {
public:
  using G4EmDNAChemistry_option1::G4EmDNAChemistry_option1;
  void ConstructDissociationChannels() override;
};

//==============================================================================
class DNAChemistryOpt2 : public DNABaseChemistry,
                         public G4EmDNAChemistry_option2 {
public:
  using G4EmDNAChemistry_option2::G4EmDNAChemistry_option2;
  void ConstructDissociationChannels() override;
};

//==============================================================================
class DNAChemistryOpt3 : public DNABaseChemistry,
                         public G4EmDNAChemistry_option3 {
public:
  using G4EmDNAChemistry_option3::G4EmDNAChemistry_option3;
  void ConstructDissociationChannels() override;
};

#endif // DNA_CHEMISTRY_H_
