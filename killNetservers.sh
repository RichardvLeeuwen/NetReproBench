#!/bin/bash
#Please fill in DAS5 username at the spot indicated with the #######

for ip in "$@"; do
  ssh #######@"$ip" pkill netserver
done
