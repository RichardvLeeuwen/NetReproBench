# python file for the masternode script
import socket, threading
import subprocess
import argparse
import os
import shutil
import time
import sys
import statistics


PORT = 5555


class benchmarkServerThread(threading.Thread):

    def __init__(self, ip, port, socket, duration, packetSize, latency):
        threading.Thread.__init__(self)
        self.ip = ip
        self.port = port
        self.socket = socket
        self.keepWorking = True
        self.allConnected = False
        self.allDone = False
        self.duration = duration
        self.packetSize = packetSize
        self.throughputResults = []
        self.throughputResult = 0
        self.latency = latency

    def run(self):
        #print("Connected with a benchmarking server")
        while self.keepWorking:
            data = self.socket.recv(1024)
            if not data:  # connection got killed
                break
            if b'ready' in data:
                #print("rdy")
                self.allConnected = True
            if b'done' in data:
                data = str(data)
                split = data.split(",")

                for splitted in split:
                    if splitted == split[0]:
                        continue
                    if splitted == split[-1]:
                        continue
                    result = splitted
                    self.throughputResults.append(int(result))
                totalBytes = 0
                for res in self.throughputResults:
                    totalBytes = totalBytes + res
                self.throughputResult = totalBytes / len(self.throughputResults)
                reply = "recv\n"
                reply = reply.encode()
                self.socket.sendall(reply)
                self.allDone = True
        self.socket.close()


    def signalStartThroughput(self):
        #print("Signalling transmitters to start")
        data = "start," + str(self.duration) + "," + str(self.packetSize) + "," + str(self.ip) + ",;"
        data = data.encode()
        self.socket.sendall(data)

    def signalStartLatency(self):
        #print("Signalling transmitters to start")
        data = "ltc," + str(self.duration) + "," + str(self.packetSize) + "," + str(self.ip) + ",;"
        data = data.encode()
        self.socket.sendall(data)

    def finishThread(self):
        self.keepWorking = False

def mergeNodeLists(transmitters, receivers):
    return list(set(transmitters+receivers))

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
    latency = False

    BenchMarkThreads = []
    connectedServers = 0
    parser = argparse.ArgumentParser()
    parser.add_argument("-masterip", required=True, type = str,  help="Ip of the masternode")
    parser.add_argument("-transmitters", required=True,  nargs='+', help="The ips for the transmitters.")
    parser.add_argument("-receivers", required=True,  nargs='+', help="The ips for the receivers.")
    parser.add_argument("-time", required=False, type=int, nargs=1, help="Duration throughput test.")
    parser.add_argument("-packetSize", required=False, type=int, nargs=1, help="Packet size for throughput test")
    parser.add_argument("-latency", required=False, action="store_true", help="Latency flag")

    args = parser.parse_args()
    masterip = args.masterip
    transmitters = args.transmitters
    receivers = args.receivers
    timeList = args.time
    packetSizeList = args.packetSize
    if args.latency:
        latency = True


    if timeList is None:
        timer = 30
    else:
        timer = timeList[0]
    if packetSizeList is None:
        packetSize = 1024
    else:
        packetSize = packetSizeList[0]

    parseIPRange(transmitters)
    parseIPRange(receivers)
    totaltransmitters = len(transmitters)
    uniqueNodes = mergeNodeLists(transmitters, receivers)
    print(transmitters)
    print(receivers)
    print("Running the benchmarking script.")
    subprocess.call(["./killClientsandServers.sh"]+uniqueNodes)
    pathCPU = "cloudBenchCPU"

    if os.path.exists(pathCPU):
        shutil.rmtree(pathCPU)
    os.makedirs(pathCPU)



    path = "cloudBenchResults"

    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path)


    print("Launching the masternode server")
    masterNodeSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    masterNodeSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    masterNodeSock.bind((masterip, PORT))
    masterNodeSock.listen(3)
    print("Launching the benchmarking servers")
    # code to remotely initiate the server nodes
    for i in transmitters:
        total = countTotalExpectedReceivers(i, receivers)
        subprocess.Popen(["./launchServerNodeDAS.sh", str(total), masterip, i])

    while (connectedServers != totaltransmitters):
        (benchmarkServer, (ip, port)) = masterNodeSock.accept()
        newBenchmarkServerThread = benchmarkServerThread(ip, port, benchmarkServer, timer, packetSize, latency)
        newBenchmarkServerThread.start()
        BenchMarkThreads.append(newBenchmarkServerThread)
        connectedServers = connectedServers + 1


    print("All benchmarking servers are connected and ready to accepting incoming clients")
    print("Launching the benchmarking clients")
    # code to remotely initiate the client nodes
    for i in receivers:
        subprocess.Popen(["./launchClientNodesDAS.sh", i]+transmitters)
    allReady = 0
    while True:
        allReady = 0
        for i in BenchMarkThreads:
            if i.allConnected:
                allReady = allReady + 1
        if allReady == totaltransmitters:
            break

    print("All benchmarking clients and servers are connected with each other, begin benchmarking")

    if not latency:
        subprocess.Popen(["./launchCPUMeasurements.sh", str(timer)]+uniqueNodes)
        for i in BenchMarkThreads:
            i.signalStartThroughput()

    if latency:
        for i in BenchMarkThreads:
            i.signalStartLatency()

    allDone = 0
    while True:
        allDone = 0
        for i in BenchMarkThreads:
            if i.allDone:
                allDone = allDone + 1
        if allDone == totaltransmitters:
            break

    if latency:
        print("Latency tests are done")
        averageThroughputAllNodes = 0

        for i in BenchMarkThreads:
            averageThroughputAllNodes = averageThroughputAllNodes + (i.throughputResult)
        averageThroughputAllNodes = averageThroughputAllNodes/totaltransmitters


        print("Average latency in nanoseconds = "+str(averageThroughputAllNodes))
        print("Average latency in miliseconds = "+str(averageThroughputAllNodes/1000000))
        allData = []
        for filename in os.listdir(path):
            with open(os.path.join(path, filename), 'r') as f:
                linesD = f.readlines()
                for d in linesD:
                    allData.append(float(d))
        minV = min(allData)
        print("Min latency in miliseconds = " + str(minV/1000000))
        maxV = max(allData)
        print("Max latency in miliseconds = " + str(maxV/1000000))
        medianV = statistics.median(allData)
        print("Median latency in miliseconds = " + str(medianV/1000000))
        allData.sort()
        halfL = int(len(allData)/2)
        firstQ = statistics.median(allData[:halfL])
        print("first Q latency in miliseconds = " + str(firstQ/1000000))
        thirdQ = statistics.median(allData[halfL:])
        print("third Q latency in miliseconds = " + str(thirdQ/1000000))


        for i in BenchMarkThreads:
            i.finishThread()
        print("Benchmark is completed")

        sys.exit()
    print("Throughput tests are done")
    averageThroughputAllNodes = 0

    for i in BenchMarkThreads:
        averageThroughputAllNodes = averageThroughputAllNodes + (i.throughputResult/int(timer))
    averageThroughputAllNodes = averageThroughputAllNodes/totaltransmitters


    print("Average throughput in bytes = "+str(averageThroughputAllNodes))
    print("Average throughput in bits = " + str(averageThroughputAllNodes*8))
    print("Average throughput in gigabit/second = " + str(((((averageThroughputAllNodes*8) /1024) /1024) /1024)))


    allData = []
    for filename in os.listdir(path):
        with open(os.path.join(path, filename), 'r') as f:
            linesD = f.readlines()
            linesD.pop(0)
            linesD.pop(0)
            temp = []
            for i in range(len(linesD)):
                d = linesD[i]
                if i == 0:
                    allData.append(float(d))
                else:
                    d2 = linesD[i-1]
                    d3 = float(d) - float(d2)
                    allData.append(d3)
    minV = min(allData)
    print("Min throughput in gigabit/second = " + str(((((minV*8) /1024) /1024) /1024)))
    maxV = max(allData)
    print("Max throughput in gigabit/second = " + str(((((maxV*8) /1024) /1024) /1024)))
    medianV = statistics.median(allData)
    print("Median throughput in gigabit/second = " + str(((((medianV*8) /1024) /1024) /1024)))
    allData.sort()
    halfL = int(len(allData)/2)
    firstQ = statistics.median(allData[:halfL])
    print("first Q throughput in gigabit/second = " + str(((((firstQ*8) /1024) /1024) /1024)))
    thirdQ = statistics.median(allData[halfL:])
    print("third Q throughput in gigabit/second = " + str(((((thirdQ*8) /1024) /1024) /1024)))





    totalFiles = 0
    totalDiff = 0
    highestDiff = -1
    nameHighDiff = "None"
    for filename in os.listdir(path):
        with open(os.path.join(path, filename), 'r') as f:
            lines = f.readlines()
            if len(lines) < 2:
                continue
            send = int(lines[0])
            recv = int(lines[1])
            diff = abs(send - recv)
            totalDiff = totalDiff + diff
            totalFiles = totalFiles + 1
            if(diff > highestDiff):
                highestDiff = diff
                nameHighDiff = filename
            elif(diff == highestDiff):
                nameHighDiff = nameHighDiff + ", " + filename

    print("Highest byte difference between sending and receiving is " + str(highestDiff))
    print("average byte difference between send and receive is " + str(totalDiff/totalFiles))

    for i in BenchMarkThreads:
        i.finishThread()
    print("Benchmark is completed")


if __name__ == "__main__":
    main()
