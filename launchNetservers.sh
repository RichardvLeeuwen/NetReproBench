#!/bin/bash
#please fill in das5 username at the spot indicated with the #######

for ip in "$@"; do
  ssh #######@"$ip" netserver &
done
