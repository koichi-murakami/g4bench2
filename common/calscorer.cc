/*============================================================================
Copyright 2022 Koichi Murakami

Distributed under the OSI-approved BSD License (the "License");
see accompanying file LICENSE for details.

This software is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the License for more information.
============================================================================*/
#include "G4Step.hh"
#include "G4Threading.hh"
#include "common/calscorer.h"
#include "common/simdata.h"

// --------------------------------------------------------------------------
CalScorer::CalScorer()
: G4VSensitiveDetector("calscorer"), simdata_{nullptr}
{
}

// --------------------------------------------------------------------------
bool CalScorer::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  auto tid = G4Threading::G4GetThreadId();

  if ( tid == G4Threading::MASTER_ID) {
    tid = 0;
  }

  auto edep = step-> GetTotalEnergyDeposit();
  simdata_[tid].AddEdep(edep);

  return true;
}
