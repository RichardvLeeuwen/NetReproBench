#!/bin/bash
#please fill in DAS5 username at the spot indicated with the #######

for ip in "$@"; do
  ssh #######@"$ip" pkill server
done

for ip in "$@"; do
  ssh #######@"$ip" pkill client
done
