/*============================================================================
Copyright 2022 Koichi Murakami

Distributed under the OSI-approved BSD License (the "License");
see accompanying file LICENSE for details.

This software is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the License for more information.
============================================================================*/
#ifndef STEP_ACTION_H_
#define STEP_ACTION_H_

#include "G4UserSteppingAction.hh"

class SimData;

class StepAction : public G4UserSteppingAction {
public:
  StepAction();
  ~StepAction() override = default;

  void SetSimData(SimData* data);

  void UserSteppingAction(const G4Step* step) override;

private:
  SimData* simdata_;

};

// ==========================================================================
inline void StepAction::SetSimData(SimData* data)
{
  simdata_ = data;
}

#endif
