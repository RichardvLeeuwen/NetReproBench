#!/bin/bash
#Please fill in DAS5 username at the spot indicated with the #######
time=$1

shift
for ip in "$@"; do
  ssh #######@"$ip" ~/thesis/MeasureCPU.sh $time $ip &
done

