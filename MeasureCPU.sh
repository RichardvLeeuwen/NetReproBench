#!/bin/bash

duration=$1
ip=$2

readarray -t allCPU < <(head -n33 /proc/stat)

declare -a all_prev_idle
declare -a all_prev_cpu_time


for ((j=0;j<33;j++))
do
  row="${allCPU[j]}"
  all_columns=($row)
  columns=("${all_columns[@]:1}")
  total_cpu_time="${columns[*]}"
  total_cpu_time=${total_cpu_time// /+}

  all_prev_idle[j]="${columns[3]}"
  all_prev_cpu_time[j]=$total_cpu_time

done

sleep $duration

readarray -t allCPU < <(head -n33 /proc/stat)


for ((j=0;j<33;j++))
do
  row="${allCPU[j]}"
  all_columns=($row)
  columns=("${all_columns[@]:1}")
  total_cpu_time="${columns[*]}"
  total_cpu_time=${total_cpu_time// /+}

  prev_total_cpu_time="${all_prev_cpu_time[j]}"
  prev_cpu_idle="${all_prev_idle[j]}"


  cpu_diff=$((total_cpu_time - prev_total_cpu_time))
  cpu_idle_diff=$((columns[3]- prev_cpu_idle))
  idle_percentage=$(((cpu_idle_diff * 100) / cpu_diff))
  cpu_usage=$((100 - idle_percentage))
  core_num=$((j-1))
  if (( $core_num == -1 ))
  then
    core_num="all"
  fi
  echo "${core_num}:${cpu_usage}" >> thesis/cloudBenchCPU/${ip}.txt
done