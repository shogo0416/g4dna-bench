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
#include "dna_dissociation_channel.h"
#include "G4Version.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4DNAWaterDissociationDisplacer.hh"
#include "G4DNAWaterExcitationStructure.hh"
#include "G4MoleculeTable.hh"
#include "G4H2O.hh"

//------------------------------------------------------------------------------
void DNADissociationChannel::ConstructDissociationChannels(
  bool alt_B1A1_decay, bool alt_decay_vibH2O)
{
  auto mtab = G4MoleculeTable::Instance();

  // Get the molecular configuration
#if G4VERSION_NUMBER >= 1130
  auto* OH   = mtab->GetConfiguration("°OH");
#else
  auto* OH   = mtab->GetConfiguration("OH");
#endif
  auto* OHm  = mtab->GetConfiguration("OHm");
  auto* e_aq = mtab->GetConfiguration("e_aq");
  auto* H2   = mtab->GetConfiguration("H2");
  auto* H3O  = mtab->GetConfiguration("H3Op");
  auto* H    = mtab->GetConfiguration("H");
  auto* O3P  = mtab->GetConfiguration("Oxy");

  // Define the decay channels
  auto water = G4H2O::Definition();
  G4MolecularDissociationChannel* decCh1{nullptr};
  G4MolecularDissociationChannel* decCh2{nullptr};
  G4MolecularDissociationChannel* decCh3{nullptr};
  G4MolecularDissociationChannel* decCh4{nullptr};
  G4MolecularDissociationChannel* decCh5{nullptr};

  auto occ = new G4ElectronOccupancy(
    *(water->GetGroundStateElectronOccupancy()));

  //////////////////////////////////////////////////////////////////////////////
  //  EXCITATIONS
  //////////////////////////////////////////////////////////////////////////////
  G4DNAWaterExcitationStructure waterExcitation;

  // ===========================================================================
  //  Excitation on the fifth layer
  // ===========================================================================
  decCh1 = new G4MolecularDissociationChannel(
            "A^1B_1_Relaxation");
  decCh2 = new G4MolecularDissociationChannel(
            "A^1B_1_DissociativeDecay");
  //Decay 1 : OH + H
  decCh1->SetEnergy(waterExcitation.ExcitationEnergy(0));
  decCh1->SetProbability(0.35);
  decCh1->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::NoDisplacement);

  decCh2->AddProduct(OH);
  decCh2->AddProduct(H);
  decCh2->SetProbability(0.65);
  decCh2->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::A1B1_DissociationDecay);

  occ->RemoveElectron(4, 1); // this is the transition form ground state to
  occ->AddElectron(5, 1);    // the first unoccupied orbital: A^1B_1

  water->NewConfigurationWithElectronOccupancy("A^1B_1", *occ);
  water->AddDecayChannel("A^1B_1", decCh1);
  water->AddDecayChannel("A^1B_1", decCh2);

  // ===========================================================================
  //  Excitation on the fourth layer
  // ===========================================================================
  if (alt_B1A1_decay) {

    //
    // Reference: J.Meesungnoen et. al, DOI: 10.1021/jp058037z
    //
    decCh1 = new G4MolecularDissociationChannel(
              "B^1A_1_Relaxation_Channel");
    decCh2 = new G4MolecularDissociationChannel(
              "B^1A_1_DissociativeDecay");
    decCh3 = new G4MolecularDissociationChannel(
              "B^1A_1_AutoIonisation_Channel");
    decCh4 = new G4MolecularDissociationChannel(
              "B^1A_1_DissociativeDecay1");
    decCh5 = new G4MolecularDissociationChannel(
              "B^1A_1_DissociativeDecay2");

    //Decay 1 : thermal energy release
    decCh1->SetEnergy(waterExcitation.ExcitationEnergy(1));
    decCh1->SetProbability(0.175);

    //Decay 2 : O(1D) + H_2 -> 2OH + H_2
    decCh2->AddProduct(H2);
    decCh2->AddProduct(OH);
    decCh2->AddProduct(OH);
    decCh2->SetProbability(0.0325);
    decCh2->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::B1A1_DissociationDecay);

    //Decay 3 : OH + H_3Op + e_aq
    decCh3->AddProduct(OH);
    decCh3->AddProduct(H3O);
    decCh3->AddProduct(e_aq);
    decCh3->SetProbability(0.50);
    decCh3->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::AutoIonisation);

    //Decay 4 :  H + OH
    decCh4->AddProduct(H);
    decCh4->AddProduct(OH);
    decCh4->SetProbability(0.2535);
    decCh4->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::A1B1_DissociationDecay);

    //Decay 5 : 2H + O(3P)
    decCh5->AddProduct(O3P);
    decCh5->AddProduct(H);
    decCh5->AddProduct(H);
    decCh5->SetProbability(0.039);
    decCh5->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::B1A1_DissociationDecay2);

    *occ = *(water->GetGroundStateElectronOccupancy());
    occ->RemoveElectron(3); // this is the transition form ground state to
    occ->AddElectron(5, 1); // the first unoccupied orbital: B^1A_1

    water->NewConfigurationWithElectronOccupancy("B^1A_1", *occ);
    water->AddDecayChannel("B^1A_1", decCh1);
    water->AddDecayChannel("B^1A_1", decCh2);
    water->AddDecayChannel("B^1A_1", decCh3);
    water->AddDecayChannel("B^1A_1", decCh4);
    water->AddDecayChannel("B^1A_1", decCh5);

  } else {

    //
    // Geant4-DNA original decay channels
    //
    decCh1 = new G4MolecularDissociationChannel(
              "B^1A_1_Relaxation_Channel");
    decCh2 = new G4MolecularDissociationChannel(
              "B^1A_1_DissociativeDecay");
    decCh3 = new G4MolecularDissociationChannel(
              "B^1A_1_AutoIonisation_Channel");

    //Decay 1 : energy
    decCh1->SetEnergy(waterExcitation.ExcitationEnergy(1));
    decCh1->SetProbability(0.3);

    //Decay 2 : 2OH + H_2
    decCh2->AddProduct(H2);
    decCh2->AddProduct(OH);
    decCh2->AddProduct(OH);
    decCh2->SetProbability(0.15);
    decCh2->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::B1A1_DissociationDecay);

    //Decay 3 : OH + H_3Op + e_aq
    decCh3->AddProduct(OH);
    decCh3->AddProduct(H3O);
    decCh3->AddProduct(e_aq);
    decCh3->SetProbability(0.55);
    decCh3->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::AutoIonisation);

    *occ = *(water->GetGroundStateElectronOccupancy());
    occ->RemoveElectron(3); // this is the transition form ground state to
    occ->AddElectron(5, 1); // the first unoccupied orbital: B^1A_1

    water->NewConfigurationWithElectronOccupancy("B^1A_1", *occ);
    water->AddDecayChannel("B^1A_1", decCh1);
    water->AddDecayChannel("B^1A_1", decCh2);
    water->AddDecayChannel("B^1A_1", decCh3);
  }

  // ===========================================================================
  //  Excitation of 3rd layer
  // ===========================================================================
  decCh1 = new G4MolecularDissociationChannel(
            "Excitation3rdLayer_AutoIonisation_Channel");
  decCh2 = new G4MolecularDissociationChannel(
            "Excitation3rdLayer_Relaxation_Channel");

  //Decay channel 1 : : OH + H_3Op + e_aq
  decCh1->AddProduct(OH);
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(e_aq);

  decCh1->SetProbability(0.5);
  decCh1->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::AutoIonisation);

  //Decay channel 2 : energy
  decCh2->SetEnergy(waterExcitation.ExcitationEnergy(2));
  decCh2->SetProbability(0.5);

  //Electronic configuration of this decay
  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->AddElectron(5, 1);

  //Configure the water molecule
  water->NewConfigurationWithElectronOccupancy("Excitation3rdLayer", *occ);
  water->AddDecayChannel("Excitation3rdLayer", decCh1);
  water->AddDecayChannel("Excitation3rdLayer", decCh2);

  // ===========================================================================
  //  Excitation of 2nd layer
  // ===========================================================================
  decCh1 = new G4MolecularDissociationChannel(
            "Excitation2ndLayer_AutoIonisation_Channel");
  decCh2 = new G4MolecularDissociationChannel(
            "Excitation2ndLayer_Relaxation_Channel");

  //Decay Channel 1 : : OH + H_3Op + e_aq
  decCh1->AddProduct(OH);
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(e_aq);

  decCh1->SetProbability(0.5);
  decCh1->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::AutoIonisation);

  //Decay channel 2 : energy
  decCh2->SetEnergy(waterExcitation.ExcitationEnergy(3));
  decCh2->SetProbability(0.5);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->AddElectron(5, 1);

  water->NewConfigurationWithElectronOccupancy("Excitation2ndLayer", *occ);
  water->AddDecayChannel("Excitation2ndLayer", decCh1);
  water->AddDecayChannel("Excitation2ndLayer", decCh2);

  // ===========================================================================
  //  Excitation of 1st layer
  // ===========================================================================
  decCh1 = new G4MolecularDissociationChannel(
            "Excitation1stLayer_AutoIonisation_Channel");
  decCh2 = new G4MolecularDissociationChannel(
            "Excitation1stLayer_Relaxation_Channel");

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->AddElectron(5, 1);

  //Decay Channel 1 : : OH + H_3Op + e_aq
  decCh1->AddProduct(OH);
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(e_aq);
  decCh1->SetProbability(0.5);
  decCh1->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::AutoIonisation);

  //Decay channel 2 : energy
  decCh2->SetEnergy(waterExcitation.ExcitationEnergy(4));
  decCh2->SetProbability(0.5);

  water->NewConfigurationWithElectronOccupancy("Excitation1stLayer", *occ);
  water->AddDecayChannel("Excitation1stLayer", decCh1);
  water->AddDecayChannel("Excitation1stLayer", decCh2);

  //////////////////////////////////////////////////////////////////////////////
  // SINGLE-IONISATION
  //////////////////////////////////////////////////////////////////////////////
  decCh1 = new G4MolecularDissociationChannel("SingleIonisation_Channel");

  //Decay Channel 1 : : OH + H_3Op
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(OH);
  decCh1->SetProbability(1);
  decCh1->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::Ionisation_DissociationDecay);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 1);
  // this is a ionized h2O with a hole in its last orbital
  water->NewConfigurationWithElectronOccupancy("SingleIonisation5", *occ);
  water->AddDecayChannel("SingleIonisation5", decCh1);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 1);
  water->NewConfigurationWithElectronOccupancy("SingleIonisation4", *occ);
  water->AddDecayChannel("SingleIonisation4",
      new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  water->NewConfigurationWithElectronOccupancy("SingleIonisation3", *occ);
  water->AddDecayChannel("SingleIonisation3",
      new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  water->NewConfigurationWithElectronOccupancy("SingleIonisation2", *occ);
  water->AddDecayChannel("SingleIonisation2",
      new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("SingleIonisation1", *occ);
  water->AddDecayChannel("SingleIonisation1",
      new G4MolecularDissociationChannel(*decCh1));

#if G4VERSION_NUMBER >= 1130
  //////////////////////////////////////////////////////////////////////////////
  // DOUBLE-IONISATION
  //////////////////////////////////////////////////////////////////////////////
  decCh1 = new G4MolecularDissociationChannel("DoubleIonisation_Channel1");
  decCh2 = new G4MolecularDissociationChannel("DoubleIonisation_Channel2");
  decCh3 = new G4MolecularDissociationChannel("DoubleIonisation_Channel3");

  // Decay Channel #1: H2O^2+ -> 2H+ + O(3P) -> 2H3O+ + O(3P)
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(O3P);
  decCh1->SetProbability(0.29);
  decCh1->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::DoubleIonisation_DissociationDecay1);

  // Decay Channel #2: H2O^2+ -> H+ + H* + O+ -> 2H3O+ + H* + *OH + O(3P)
  decCh2->AddProduct(H3O);
  decCh2->AddProduct(H3O);
  decCh2->AddProduct(H);
  decCh2->AddProduct(OH);
  decCh2->AddProduct(O3P);
  decCh2->SetProbability(0.16);
  decCh2->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::DoubleIonisation_DissociationDecay2);

  // Decay Channel #3: H2O^2+ -> H+ + OH+ -> 2H3O+ + O(3P)
  decCh3->AddProduct(H3O);
  decCh3->AddProduct(H3O);
  decCh3->AddProduct(O3P);
  decCh3->SetProbability(0.55);
  decCh3->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::DoubleIonisation_DissociationDecay3);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation15", *occ);
  water->AddDecayChannel("DoubleIonisation15", decCh1);
  water->AddDecayChannel("DoubleIonisation15", decCh2);
  water->AddDecayChannel("DoubleIonisation15", decCh3);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation14", *occ);
  water->AddDecayChannel("DoubleIonisation14",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation14",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation14",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation13", *occ);
  water->AddDecayChannel("DoubleIonisation13",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation13",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation13",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation12", *occ);
  water->AddDecayChannel("DoubleIonisation12",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation12",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation12",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(3, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation11", *occ);
  water->AddDecayChannel("DoubleIonisation11",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation11",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation11",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 2);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation10", *occ);
  water->AddDecayChannel("DoubleIonisation10",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation10",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation10",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation9", *occ);
  water->AddDecayChannel("DoubleIonisation9",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation9",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation9",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(3, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation8", *occ);
  water->AddDecayChannel("DoubleIonisation8",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation8",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation8",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(2, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation7", *occ);
  water->AddDecayChannel("DoubleIonisation7",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation7",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation7",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 2);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation6", *occ);
  water->AddDecayChannel("DoubleIonisation6",
                        new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation6",
                        new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation6",
                        new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation5", *occ);
  water->AddDecayChannel("DoubleIonisation5",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation5",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation5",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(3, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation4", *occ);
  water->AddDecayChannel("DoubleIonisation4",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation4",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation4",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(2, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation3", *occ);
  water->AddDecayChannel("DoubleIonisation3",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation3",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation3",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(1, 1);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation2", *occ);
  water->AddDecayChannel("DoubleIonisation2",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation2",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation2",
                         new G4MolecularDissociationChannel(*decCh3));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 2);
  water->NewConfigurationWithElectronOccupancy("DoubleIonisation1", *occ);
  water->AddDecayChannel("DoubleIonisation1",
                         new G4MolecularDissociationChannel(*decCh1));
  water->AddDecayChannel("DoubleIonisation1",
                         new G4MolecularDissociationChannel(*decCh2));
  water->AddDecayChannel("DoubleIonisation1",
                         new G4MolecularDissociationChannel(*decCh3));

  //////////////////////////////////////////////////////////////////////////////
  // TRIPLE-IONISATION
  //////////////////////////////////////////////////////////////////////////////
  decCh1 = new G4MolecularDissociationChannel("TripleIonisation_Channel");

  //Decay Channel 1 : H2O^3+ -> 3H_3Op + OH + O(3P)
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(OH);
  decCh1->AddProduct(O3P);
  decCh1->SetProbability(1.0);
  decCh1->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::TripleIonisation_DissociationDecay);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(2, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation30", *occ);
  water->AddDecayChannel("TripleIonisation30", decCh1);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(1, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation29", *occ);
  water->AddDecayChannel("TripleIonisation29",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(1, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation28", *occ);
  water->AddDecayChannel("TripleIonisation28",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(1, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation27", *occ);
  water->AddDecayChannel("TripleIonisation27",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation26", *occ);
  water->AddDecayChannel("TripleIonisation26",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation25", *occ);
  water->AddDecayChannel("TripleIonisation25",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation24", *occ);
  water->AddDecayChannel("TripleIonisation24",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation23", *occ);
  water->AddDecayChannel("TripleIonisation23",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation22", *occ);
  water->AddDecayChannel("TripleIonisation22",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation21", *occ);
  water->AddDecayChannel("TripleIonisation21",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 2);
  occ->RemoveElectron(3, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation20", *occ);
  water->AddDecayChannel("TripleIonisation20",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 2);
  occ->RemoveElectron(2, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation19", *occ);
  water->AddDecayChannel("TripleIonisation19",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 2);
  occ->RemoveElectron(1, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation18", *occ);
  water->AddDecayChannel("TripleIonisation18",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(4, 2);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation17", *occ);
  water->AddDecayChannel("TripleIonisation17",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 2);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation16", *occ);
  water->AddDecayChannel("TripleIonisation16",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 2);
  occ->RemoveElectron(2, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation15", *occ);
  water->AddDecayChannel("TripleIonisation15",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 2);
  occ->RemoveElectron(1, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation14", *occ);
  water->AddDecayChannel("TripleIonisation14",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 2);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation13", *occ);
  water->AddDecayChannel("TripleIonisation13",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 2);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation12", *occ);
  water->AddDecayChannel("TripleIonisation12",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 2);
  occ->RemoveElectron(3, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation11", *occ);
  water->AddDecayChannel("TripleIonisation11",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 2);
  occ->RemoveElectron(1, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation10", *occ);
  water->AddDecayChannel("TripleIonisation10",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 2);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation9", *occ);
  water->AddDecayChannel("TripleIonisation9",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 2);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation8", *occ);
  water->AddDecayChannel("TripleIonisation8",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 2);
  occ->RemoveElectron(3, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation7", *occ);
  water->AddDecayChannel("TripleIonisation7",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 2);
  occ->RemoveElectron(2, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation6", *occ);
  water->AddDecayChannel("TripleIonisation6",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 2);
  occ->RemoveElectron(0, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation5", *occ);
  water->AddDecayChannel("TripleIonisation5",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 2);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation4", *occ);
  water->AddDecayChannel("TripleIonisation4",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 2);
  occ->RemoveElectron(3, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation3", *occ);
  water->AddDecayChannel("TripleIonisation3",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 2);
  occ->RemoveElectron(2, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation2", *occ);
  water->AddDecayChannel("TripleIonisation2",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 2);
  occ->RemoveElectron(1, 1);
  water->NewConfigurationWithElectronOccupancy("TripleIonisation1", *occ);
  water->AddDecayChannel("TripleIonisation1",
                         new G4MolecularDissociationChannel(*decCh1));

  //////////////////////////////////////////////////////////////////////////////
  // QUADRUPLE-IONISATION
  //////////////////////////////////////////////////////////////////////////////
  decCh1 = new G4MolecularDissociationChannel("QuadrupleIonisation_Channel");

  //Decay Channel 1 : H2O^4+ -> 4H_3Op + 2OH + O(3P)
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(H3O);
  decCh1->AddProduct(OH);
  decCh1->AddProduct(OH);
  decCh1->AddProduct(O3P);
  decCh1->SetProbability(1.0);
  decCh1->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::QuadrupleIonisation_DissociationDecay);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(3, 1);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation1", *occ);
  water->AddDecayChannel("QuadrupleIonisation1", decCh1);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation2", *occ);
  water->AddDecayChannel("QuadrupleIonisation2",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation3", *occ);
  water->AddDecayChannel("QuadrupleIonisation3",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation4", *occ);
  water->AddDecayChannel("QuadrupleIonisation4",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 1);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation5", *occ);
  water->AddDecayChannel("QuadrupleIonisation5",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(2, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation6", *occ);
  water->AddDecayChannel("QuadrupleIonisation6",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation7", *occ);
  water->AddDecayChannel("QuadrupleIonisation7",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation8", *occ);
  water->AddDecayChannel("QuadrupleIonisation8",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(1, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation9", *occ);
  water->AddDecayChannel("QuadrupleIonisation9",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation10", *occ);
  water->AddDecayChannel("QuadrupleIonisation10",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation11", *occ);
  water->AddDecayChannel("QuadrupleIonisation11",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(1, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation12", *occ);
  water->AddDecayChannel("QuadrupleIonisation12",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(2, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation13", *occ);
  water->AddDecayChannel("QuadrupleIonisation13",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation14", *occ);
  water->AddDecayChannel("QuadrupleIonisation14",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(1, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation15", *occ);
  water->AddDecayChannel("QuadrupleIonisation15",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(2, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation16", *occ);
  water->AddDecayChannel("QuadrupleIonisation16",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation17", *occ);
  water->AddDecayChannel("QuadrupleIonisation17",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(0, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation18", *occ);
  water->AddDecayChannel("QuadrupleIonisation18",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation19", *occ);
  water->AddDecayChannel("QuadrupleIonisation19",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation20", *occ);
  water->AddDecayChannel("QuadrupleIonisation20",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(0, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation21", *occ);
  water->AddDecayChannel("QuadrupleIonisation21",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(2, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation22", *occ);
  water->AddDecayChannel("QuadrupleIonisation22",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation23", *occ);
  water->AddDecayChannel("QuadrupleIonisation23",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(0, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation24", *occ);
  water->AddDecayChannel("QuadrupleIonisation24",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(2, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation25", *occ);
  water->AddDecayChannel("QuadrupleIonisation25",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation26", *occ);
  water->AddDecayChannel("QuadrupleIonisation26",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(0, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation27", *occ);
  water->AddDecayChannel("QuadrupleIonisation27",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(1, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation28", *occ);
  water->AddDecayChannel("QuadrupleIonisation28",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation29", *occ);
  water->AddDecayChannel("QuadrupleIonisation29",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(0, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation30", *occ);
  water->AddDecayChannel("QuadrupleIonisation30",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(1, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation31", *occ);
  water->AddDecayChannel("QuadrupleIonisation31",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation32", *occ);
  water->AddDecayChannel("QuadrupleIonisation32",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(0, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation33", *occ);
  water->AddDecayChannel("QuadrupleIonisation33",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(1, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation34", *occ);
  water->AddDecayChannel("QuadrupleIonisation34",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 1);
  occ->RemoveElectron(4, 1);
  occ->RemoveElectron(2, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation35", *occ);
  water->AddDecayChannel("QuadrupleIonisation35",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 2);
  occ->RemoveElectron(1, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation36", *occ);
  water->AddDecayChannel("QuadrupleIonisation36",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 2);
  occ->RemoveElectron(2, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation37", *occ);
  water->AddDecayChannel("QuadrupleIonisation37",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 2);
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation38", *occ);
  water->AddDecayChannel("QuadrupleIonisation38",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(0, 2);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation39", *occ);
  water->AddDecayChannel("QuadrupleIonisation39",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 2);
  occ->RemoveElectron(2, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation40", *occ);
  water->AddDecayChannel("QuadrupleIonisation40",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 2);
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation41", *occ);
  water->AddDecayChannel("QuadrupleIonisation41",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(1, 2);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation42", *occ);
  water->AddDecayChannel("QuadrupleIonisation42",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 2);
  occ->RemoveElectron(3, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation43", *occ);
  water->AddDecayChannel("QuadrupleIonisation43",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(2, 2);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation44", *occ);
  water->AddDecayChannel("QuadrupleIonisation44",
                         new G4MolecularDissociationChannel(*decCh1));

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->RemoveElectron(3, 2);
  occ->RemoveElectron(4, 2);
  water->NewConfigurationWithElectronOccupancy("QuadrupleIonisation45", *occ);
  water->AddDecayChannel("QuadrupleIonisation45",
                         new G4MolecularDissociationChannel(*decCh1));
#endif // G4VERSION_NUMBER >= 1130

  //////////////////////////////////////////////////////////////////////////////
  // Dissociative Attachment
  //////////////////////////////////////////////////////////////////////////////
  decCh1 = new G4MolecularDissociationChannel("DissociativeAttachment");

  //Decay 1 : 2OH + H_2
  decCh1->AddProduct(H2);
  decCh1->AddProduct(OHm);
  decCh1->AddProduct(OH);
  decCh1->SetProbability(1);
  decCh1->SetDisplacementType(
    G4DNAWaterDissociationDisplacer::DissociativeAttachment);

  *occ = *(water->GetGroundStateElectronOccupancy());
  occ->AddElectron(5, 1); // H_2O^-
  water->NewConfigurationWithElectronOccupancy("DissociativeAttachment", *occ);
  water->AddDecayChannel("DissociativeAttachment", decCh1);

  //////////////////////////////////////////////////////////////////////////////
  // Electron-hole Recombination
  //////////////////////////////////////////////////////////////////////////////
  const auto pH2Ovib = G4H2O::Definition()->NewConfiguration("H2Ovib");

  if (alt_decay_vibH2O) {

    //
    // Reference: J.Meesungnoen et. al, DOI: 10.1021/jp058037z
    //
    decCh1 = new G4MolecularDissociationChannel("H2Ovib_DissociDecay1");
    decCh2 = new G4MolecularDissociationChannel("H2Ovib_DissociDecay2");
    decCh3 = new G4MolecularDissociationChannel("H2Ovib_DissociDecay3");
    decCh4 = new G4MolecularDissociationChannel("H2Ovib_DissociDecay4");

    // Decay 1 : O(1D) + H_2 -> 2OH + H_2
    decCh1->AddProduct(H2);
    decCh1->AddProduct(OH);
    decCh1->AddProduct(OH);
    decCh1->SetProbability(0.09555);
    decCh1->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::B1A1_DissociationDecay);

    // Decay 2 : 2H + O(3P)
    decCh1->AddProduct(H);
    decCh1->AddProduct(H);
    decCh1->AddProduct(O3P);
    decCh1->SetProbability(0.03575);
    decCh1->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::B1A1_DissociationDecay2);

    // Decay 3 : OH + H
    decCh3->AddProduct(OH);
    decCh3->AddProduct(H);
    decCh3->SetProbability(0.5187);
    decCh3->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::A1B1_DissociationDecay);

    // Decay 4 : relaxation
    decCh4->SetProbability(0.35);

    water->AddDecayChannel(pH2Ovib, decCh1);
    water->AddDecayChannel(pH2Ovib, decCh2);
    water->AddDecayChannel(pH2Ovib, decCh3);
    water->AddDecayChannel(pH2Ovib, decCh4);

  } else {

    //
    // Geant4-DNA original decay channels
    //
    decCh1 = new G4MolecularDissociationChannel("H2Ovib_DissociDecay1");
    decCh2 = new G4MolecularDissociationChannel("H2Ovib_DissociDecay2");
    decCh3 = new G4MolecularDissociationChannel("H2Ovib_DissociDecay3");

    // Decay 1 : O(1D) + H_2 -> 2OH + H_2
    decCh1->AddProduct(H2);
    decCh1->AddProduct(OH);
    decCh1->AddProduct(OH);
    decCh1->SetProbability(0.15);
    decCh1->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::B1A1_DissociationDecay);

    // Decay 2 : OH + H
    decCh2->AddProduct(OH);
    decCh2->AddProduct(H);
    decCh2->SetProbability(0.55);
    decCh2->SetDisplacementType(
      G4DNAWaterDissociationDisplacer::A1B1_DissociationDecay);

    // Decay 3 : relaxation
    decCh3->SetProbability(0.30);

    water->AddDecayChannel(pH2Ovib, decCh1);
    water->AddDecayChannel(pH2Ovib, decCh2);
    water->AddDecayChannel(pH2Ovib, decCh3);

  }

  delete occ;
}
