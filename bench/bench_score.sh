#!/bin/sh -
# ======================================================================
#  G4Bench score
# ======================================================================
export LANG=C

# ======================================================================
# run parameters
# ======================================================================
export G4DATA=/opt/geant4/data

# ======================================================================
# help message
# ======================================================================
show_help() {
cat <<EOF

\`bench_score.sh' run benchmark score.

Usage: bench_score.sh [OPTION]

Options:
  -h, --help                display this help and exit
EOF
}

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
# parsing options
while test $# -gt 0
do
  case $1 in
    -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
    *) optarg= ;;
  esac

  case $1 in
    --help|-h) show_help;  exit 0 ;;
    # ---------------------------------------------------------------
  esac
  shift
done

#
sys=`uname`
if [ ${sys} = "Darwin" ]; then
  cpu_info=`sysctl machdep.cpu.brand_string | cut -d : -f 2 | xargs echo`
else
  cpu_info=`lscpu | grep name | cut -d : -f 2 | xargs echo`
fi

#
if [ -d run ]; then
  echo "[ERROR] run directory already exists."
  exit -1
fi
mkdir run
cd run

show_line
echo " Execute G4Bench score"
show_line
echo "** CPU : $cpu_info"

# ======================================================================
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

#
NPROCS=`nproc`
NEVENTS=20000
NEVENTS_PAR=`expr $NEVENTS \* $NPROCS`
LOG_SINGLE=ecal_e1000.log
LOG_MT=ecal_e1000-mt.log

# single thread
../../ecal/ecal -b ecal_e1000 -p "${cpu_info}" ${NEVENTS} > ${LOG_SINGLE} 2>&1
eps=`cat g4bench.json | jq ".eps"`
score_single=`echo $eps \* 1000 | bc`
echo "** Single Score = $score_single"


# multi-thread thread
../../ecal/ecal -b ecal_e1000 -p "${cpu_info}" -n ${NPROCS} ${NEVENTS_PAR} > ${LOG_MT} 2>&1
eps=`cat g4bench.json | jq ".eps"`
score_mt=`echo $eps \* 1000 | bc`
echo "** MT Score = $score_mt ($NPROCS)"

# ======================================================================
cd ..
exit $?
