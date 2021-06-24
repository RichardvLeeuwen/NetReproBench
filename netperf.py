# python file for the masternode script
import socket, threading
import subprocess
import argparse
import time
import os
import shutil


def countTotalExpectedReceivers(transmitter, receivers):
    total = 0
    for recv in receivers:
        if recv != transmitter:
            total += 1
    return total

def parseIPRange(IPList):
    ipRangeSubString = "-"
    newIPS = []
    removedIPRanges = []
    for i in IPList:
        if ipRangeSubString in i:
            splitIP = i.rsplit(".", 1)
            IPfirsthalf = splitIP[0]
            IPrange = splitIP[1]
            IPRangeSplit = IPrange.split("-")
            IPStart = IPRangeSplit[0]
            IPEnd = IPRangeSplit[1]
            removedIPRanges.append(i)
            for j in range(int(IPStart), int(IPEnd)+1):
                newIP = IPfirsthalf + "." +str(j)
                newIPS.append(newIP)
    IPList.extend(newIPS)
    for i in removedIPRanges:
        IPList.remove(i)

def main():
    global connectedServers
    global BenchMarkThreads
    global masterip

    path = "netperfResults"

    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path)


    BenchMarkThreads = []
    connectedServers = 0
    parser = argparse.ArgumentParser()
    parser.add_argument("-transmitters", required=True,  nargs='+', help="The ips for the transmitters.")
    parser.add_argument("-receivers", required=True,  nargs='+', help="The ips for the receivers.")

    args = parser.parse_args()
    transmitters = args.transmitters
    receivers = args.receivers
    print(transmitters)
    print(receivers)

    parseIPRange(transmitters)
    parseIPRange(receivers)
    totaltransmitters = len(transmitters)
    print(transmitters)
    print(receivers)
    print("Launching netserver command in every node.")

    # code to remotely launch netserver
    subprocess.Popen(["./launchNetservers.sh"]+receivers) #change to receivers

    time.sleep(5)
    print("launching netperf")

    for i in transmitters:
        subprocess.Popen(["./launchNetperf.sh", i]+receivers) #change to receivers

    print("waiting results...")

    time.sleep(125)



    allValues = []
    for filename in os.listdir(path):
        with open(os.path.join(path, filename), 'r') as f:
            lines = f.readlines()
            splitLine = lines[0].split()
            print(splitLine)
            value = splitLine[-1]
            allValues.append(value)


    print(allValues)

    avg = 0
    for v in allValues:
        avg = avg + float(v)
    avg = avg / len(allValues)

    avg = (avg * 1000000)
    avg = (avg/1024)
    avg = (avg/1024)
    avg = (avg/1024)

    print("Average throughput is " + str(avg) + " gigabit/sec")

    print("Launched all")

    subprocess.Popen(["./killNetservers.sh"]+receivers)

    print("killed processes")

if __name__ == "__main__":
    main()
