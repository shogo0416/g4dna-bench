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
#include "dna_chemistry.h"

#if G4VERSION_NUMBER >= 1100
#include "dna_dissociation_channel.h"
#include "G4PhysicsConstructorFactory.hh"

G4_DECLARE_PHYSCONSTR_FACTORY(DNAChemistry);
G4_DECLARE_PHYSCONSTR_FACTORY(DNAChemistryOpt1);
G4_DECLARE_PHYSCONSTR_FACTORY(DNAChemistryOpt2);
G4_DECLARE_PHYSCONSTR_FACTORY(DNAChemistryOpt3);

//------------------------------------------------------------------------------
void DNAChemistry::ConstructDissociationChannels()
{
  DNADissociationChannel::ConstructDissociationChannels(
    use_alt_B1A1_decay_, use_alt_decay_vibH2O_);
}

//------------------------------------------------------------------------------
void DNAChemistryOpt1::ConstructDissociationChannels()
{
  DNADissociationChannel::ConstructDissociationChannels(
    use_alt_B1A1_decay_, use_alt_decay_vibH2O_);
}

//------------------------------------------------------------------------------
void DNAChemistryOpt2::ConstructDissociationChannels()
{
  DNADissociationChannel::ConstructDissociationChannels(
    use_alt_B1A1_decay_, use_alt_decay_vibH2O_);
}

//------------------------------------------------------------------------------
void DNAChemistryOpt3::ConstructDissociationChannels()
{
  DNADissociationChannel::ConstructDissociationChannels(
    use_alt_B1A1_decay_, use_alt_decay_vibH2O_);
}

#endif // G4VERSION_NUMBER >= 1100
