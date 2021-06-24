#!/bin/bash
#please fill in DAS5 username at the spot indicated with the #######
receiver=$1

ipsToConnect=""

shift
for ip in "$@"; do
  if [[ "$receiver" != "$ip" ]]
  then
    ipsToConnect+="netperf -l 120 -H ${ip} | tail -1 | awk '{print $NF}' > thesis/netperfResults/${receiver}to${ip}.txt & "
  fi
done

ssh #######@"$receiver" $ipsToConnect
