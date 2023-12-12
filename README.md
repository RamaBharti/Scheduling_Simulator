# 
 This code build a discrete-event process scheduling simulator for a 
general OS. It has following properties 
 1. Processes are generated randomly in time. The inter-arrival time between two processes is 
chosen from an exponential distribution, whose meantime can be set as a parameter of the 
simulator.
2. Each process has the following attributes 
 a. PID b. Time generated c. Expected time to completion
3. The following scheduling strategies are implemented to organize process execution
a. First come first serve
b. Round robin (with parametrized time-slice)
c. Shortest Job first 
d. Shortest remaining time first
