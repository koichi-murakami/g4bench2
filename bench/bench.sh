#!/bin/sh -
# ======================================================================
#  G4Bench benchmark
# ======================================================================
export LANG=C

# ======================================================================
# run parameters
# ======================================================================
app=$1
uevent=$2
log=$3

sys=`uname`
if [ ${sys} = "Darwin" ]; then
  cpu_info=`sysctl machdep.cpu.brand_string | cut -d : -f 2 | xargs echo`
else
  cpu_info=`lscpu | grep name | cut -d : -f 2 | xargs echo`
fi

# ======================================================================
NCORES=`nproc`

if [ $NCORES = "192" ]; then
  th_list=(1 2 4 8 16 24 32 40 48 64 80 96 144 192)

elif [ $NCORES = "256" ]; then
  th_list=(1 2 4 8 16 24 32 40 48 64 80 96 112 128 192 256)

elif [ $NCORES = "128" ]; then
  th_list=(1 2 4 8 16 24 32 40 48 56 64 96 128)

elif [ $NCORES = "96" ]; then
  th_list=(1 2 4 8 16 24 32 40 48 72 96)

elif [ $NCORES = "80" ]; then
  th_list=(1 2 4 8 16 24 32 40 60 80)

elif [ $NCORES = "72" ]; then
  th_list=(1 2 4 8 16 24 36 54 72)

elif [ $NCORES = "64" ]; then
  th_list=(1 2 4 8 16 24 32 48 64)

elif [ $NCORES = "56" ]; then
  th_list=(1 2 4 8 16 24 28 42 56)

elif [ $NCORES = "48" ]; then
  th_list=(1 2 4 8 16 24 36 48)

elif [ $NCORES = "40" ]; then
  th_list=(1 2 4 8 12 20 30 40)

elif [ $NCORES = "36" ]; then
  th_list=(1 2 4 8 12 18 24 36)

elif [ $cpu_info = "Apple M1 Ultra" ]; then
  th_list=(1 2 4 8 12 16 20)

elif [ $NCORES= "24" ]; then
  th_list=(1 2 4 8 12 18 24)

else
  th_list=(1 2)
fi

#th_list=()

# ======================================================================
for t in "${th_list[@]}"
do
  echo "running... #threads = $t"
  nevent=`expr $uevent \* $t`
  $app -n $t $nevent -p "${cpu_info}" -b $log > $log-n$t.log 2>&1
  mv g4bench.json $log-n$t.json
done

cat $log-n*.json | jq -s . > $log.json
