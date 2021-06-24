#!/bin/bash
#please fill in DAS5 username at the spot indicated with the #####
receiver=$1

ipsToConnect=""

shift
for ip in "$@"; do
  if [[ "$receiver" != "$ip" ]]
  then
    ipsToConnect+="${ip} "
  fi
done

ssh #######@"$receiver" ~/thesis/client $ipsToConnect &

