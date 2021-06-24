# NetReproBench
Bachelor thesis project NetReproBench

Run the benchmarking server with: ./server [number of expected clients] <br>
Run the benchmarking client with: ./client [list of ips] <br>
Run the benchmarking script with: python3 masternode.py -transmitters [list of ips such as 127.0.0.1 127.0.0.2 etc] -receivers [list of ips such as 127.0.0.1 127.0.0.2 etc] -masterip [ip of the head node] <br>

transmitters: expects a list of ips such as 127.0.0.1 127.0.0.2 etc for the transmitters. A transmitters is responsible for transmitting data. <br>
receivers: expects a list of ips such as 127.0.0.1 127.0.0.2 etc for the receivers. A receiver is responsible for receiving data. <br>
masterip: expects the ip of the head node <br>
time: duration throughput test, iterations latency test, default is 30 <br>
packetSize: size packet during throughput/latency test, default is 1024 <br>
latency: latency flag to specify latency test, default is throughput test
-h, --help: display help tool, expected arguments <br>

An easy way to specify a range of ips is to do 127.0.0.1-10, it will then include the list of ips ranging from 127.0.0.1 to and included 127.0.0.10 <br>

In this context a transmitter transmits data and a receiver receives data. A node can be both a transmitter and a receiver which is the case in scenarios such as all-to-all.<br>
In all-to-one you have a single receiver accompanied by transmitters. In one-to-all you have a single transmitter accompanied by receivers. In all-to-all every node is both a transmitter and receiver.<br>

The python script will invoke remotely the the transmitting and receiving nodes. Once all nodes are running and connected, it will remotely instruct the nodes to start benchmarking.<br>

The server files refers to the transmitter application, the client files refer to the receiver application. <br>

Currently, the transmitters and receivers each will create a new thread for each new connection.<br>

At 32 nodes, the latency test may occassionaly experience connection issues. This is a known bug and requires restarting the script worst case.<br>

In the SSH files, please fill in your own personal DAS5 username. The script assumes all files are stored in a folder called 'thesis'.<br>
