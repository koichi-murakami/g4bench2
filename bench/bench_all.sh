#!/bin/sh -
# ======================================================================
#  G4Bench benchmark
# ======================================================================
export LANG=C

# ======================================================================
# run parameters
# ======================================================================
G4DATA=/opt/geant4/data

UNIT_NEVENTS_ECAL=20000
UNIT_NEVENTS_HCAL=4000
UNIT_NEVENTS_VGEO_X=500000
UNIT_NEVENTS_VGEO_P=200000

run_ecal=1
run_hcal=1
run_vgeo_x18=1
run_vgeo_p200=1

# ======================================================================
# functions
# ======================================================================
check_error() {
  if [ $? -ne 0 ]; then
    exit -1
  fi
}

show_line() {
echo "========================================================================"
}

# ======================================================================
# main
# ======================================================================
if [ -d run ]; then
  echo "[ERROR] prun directory already exists."
  exit -1
fi
mkdir run
cd run

show_line
echo " Execute G4Bench tests"
show_line

# ======================================================================
# ecal
show_line
echo " G4Bench / Ecal e1000"
show_line
#
cat << EOD > g4bench.conf
{
  Run : {
    Seed : 123456789,
    G4DATA : "${G4DATA}"
  },
  Primary : {
    particle  : "e-",
    energy    : 1000.0,   // MeV
    position  : [ 0., 0., -45. ],  // cm
    direction : [ 0., 0., 1.],
  }
}
EOD

if [ ${run_ecal} = 1 ]; then
  ../bench.sh ../../ecal/ecal ${UNIT_NEVENTS_ECAL} ecal_e1000
fi

# ======================================================================
# hcal
show_line
echo " G4Bench / Hcal p10"
show_line
#
cat << EOD > g4bench.conf
{
  Run : {
    Seed : 123456789,
    G4DATA : "${G4DATA}"
  },
  Primary : {
    particle  : "proton",
    energy    : 10000.0,   // MeV
    position  : [ 0., 0., -70. ],  // cm
    direction : [ 0., 0., 1.],
  }
}
EOD

if [ ${run_hcal} = 1 ]; then
  ../bench.sh ../../hcal/hcal ${UNIT_NEVENTS_HCAL} hcal_p10
fi

# ======================================================================
# vgeo-x18
show_line
echo " G4Bench / Vgeo x18"
show_line
#
cat << EOD > g4bench.conf
{
  Run : {
    Seed : 123456789,
    //SeedOnece : true,
    G4DATA : "${G4DATA}"
  },
  Primary : {
    type : "beam",
    Beam : {
      particle  : "gamma",
      photon_voltage : 18,   // photon voltate, [6,18] MV for x-ray beam
      ssd : 100.,  // SSD (cm)
      field_size : 10.0,   // field size (X/Y) in cm
    }
  }
}
EOD

if [ ${run_vgeo_x18} = 1 ]; then
  ../bench.sh ../../vgeo/vgeo ${UNIT_NEVENTS_VGEO_X} vgeo_x18
fi

# ======================================================================
# vgeo-p200
show_line
echo " G4Bench / Vgeo p200"
show_line
#
cat << EOD > g4bench.conf
{
  Run : {
    Seed : 123456789,
    SeedOnce : true,
    G4DATA : "${G4DATA}"
  },
  Primary : {
    type : "beam",
    Beam : {
      particle  : "proton",
      energy    : 200.0,   // MeV
      ssd : 100.,  // SSD (cm)
      field_size : 10.0,   // field size (X/Y) in cm
    }
  }
}
EOD

if [ ${run_vgeo_p200} = 1 ]; then
  ../bench.sh ../../vgeo/vgeo ${UNIT_NEVENTS_VGEO_P} vgeo_p200
fi

# ======================================================================
cd ..
exit $?
