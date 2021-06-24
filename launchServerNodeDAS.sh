#!/bin/bash
#please fill in DAS5 username at the spot indicated with the #######
total=$1
master=$2
ip=$3


ssh #######@"$ip"  ~/thesis/server $total $master &
