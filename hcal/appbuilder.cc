/*============================================================================
Copyright 2022 Koichi Murakami

Distributed under the OSI-approved BSD License (the "License");
see accompanying file LICENSE for details.

This software is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the License for more information.
============================================================================*/
#include <vector>
#include "FTFP_BERT.hh"
#include "G4Navigator.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "hcalgeom.h"
#include "common/appbuilder.h"
#include "common/eventaction.h"
#include "common/particlegun.h"
#include "common/runaction.h"
#include "common/simdata.h"
#include "common/stepaction.h"
#include "util/jsonparser.h"

using namespace kut;

// --------------------------------------------------------------------------
namespace {

G4RunManager* run_manager {nullptr};
JsonParser* jparser {nullptr};

// --------------------------------------------------------------------------
void SetupGeomtry(SimData* data)
{
  auto geom = new HcalGeom();
  geom-> SetSimData(data);
  ::run_manager-> SetUserInitialization(geom);
}

// --------------------------------------------------------------------------
G4ThreeVector GetPrimaryPosition()
{
  auto pos = G4ThreeVector();
  if ( ::jparser-> Contains("Primary/position") ) {
    std::vector<double> dvec;
    dvec.clear();
    ::jparser-> GetDoubleArray("Primary/position", dvec);
    pos = G4ThreeVector(dvec[0]*cm, dvec[1]*cm, dvec[2]*cm);
  }
  return pos;
}

// --------------------------------------------------------------------------
void SetupParticleGun(ParticleGun* pga)
{
  auto gun = pga-> GetGun();

  auto pname = ::jparser-> GetStringValue("Primary/particle");
  auto ptable = G4ParticleTable::GetParticleTable();
  auto pdef = ptable-> FindParticle(pname);
  if ( pdef != nullptr ) gun-> SetParticleDefinition(pdef);

  auto pkin = ::jparser-> GetDoubleValue("Primary/energy");
  gun-> SetParticleEnergy(pkin*MeV);

  std::vector<double> dvec;
  if ( ::jparser-> Contains("Primary/direction") ) {
    dvec.clear();
    ::jparser-> GetDoubleArray("Primary/direction", dvec);
    G4ThreeVector pvec(dvec[0], dvec[1], dvec[2]);
    gun-> SetParticleMomentumDirection(pvec);
  }

  auto pos = GetPrimaryPosition();
  gun-> SetParticlePosition(pos);
}

} // end of namespace

// ==========================================================================
AppBuilder::AppBuilder()
  : simdata_{nullptr}, nvec_{0}, qtest_{false},
    bench_name_{""}, cpu_name_{""}
{
  ::jparser = JsonParser::GetJsonParser();
}

// --------------------------------------------------------------------------
AppBuilder::~AppBuilder()
{
  delete [] simdata_;
}

// --------------------------------------------------------------------------
void AppBuilder::BuildApplication(int nthreads)
{
  ::run_manager = G4RunManager::GetRunManager();

  nvec_ = nthreads;

  simdata_ = new SimData[nvec_];

  ::SetupGeomtry(simdata_);
  ::run_manager-> SetUserInitialization(new FTFP_BERT);
  ::run_manager-> SetUserInitialization(this);

  long seed { 0L };
  if ( jparser-> Contains("Run/Seed") ) {
    seed = jparser-> GetLongValue("Run/Seed");
  }

  G4Random::setTheSeed(seed);

  ::run_manager-> Initialize();
}

// --------------------------------------------------------------------------
void AppBuilder::Build() const
{
  auto pga = new ParticleGun();
  ::SetupParticleGun(pga);
  SetUserAction(pga);

  auto runaction = new RunAction();
  runaction-> SetSimData(simdata_);
  runaction-> SetDataSize(nvec_);
  runaction-> SetTestingFlag(qtest_);
  runaction-> SetBenchName(bench_name_);
  runaction-> SetCPUName(cpu_name_);
  runaction-> SetNThreads(nvec_);
  SetUserAction(runaction);

  SetUserAction(new EventAction);

  auto stepaction = new StepAction;
  stepaction-> SetSimData(simdata_);
  SetUserAction(stepaction);
}

// --------------------------------------------------------------------------
void AppBuilder::BuildForMaster() const
{
  auto runaction = new RunAction();
  runaction-> SetSimData(simdata_);
  runaction-> SetDataSize(nvec_);
  runaction-> SetTestingFlag(qtest_);
  runaction-> SetBenchName(bench_name_);
  runaction-> SetCPUName(cpu_name_);
  runaction-> SetNThreads(nvec_);

  SetUserAction(runaction);
}
