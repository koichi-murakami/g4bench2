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
#include "common/simdata.h"
#include "common/stepaction.h"

// --------------------------------------------------------------------------
StepAction::StepAction()
  : simdata_{nullptr}
{
}

// --------------------------------------------------------------------------
void StepAction::UserSteppingAction(const G4Step* step)
{
  auto tid = G4Threading::G4GetThreadId();

  if ( tid == G4Threading::MASTER_ID) {
    tid = 0;
  }

  simdata_[tid].AddStepCount();
}
