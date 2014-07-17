Simulating complex network system.

Network system diagram:
                                            _
                                  _ _ _ _  | |
  Px ---->                 --->  |_|_|_|_| |_|
Time:5s   \  _ _ _ _     /                 S1, Time:4s
            |_|_|_|_| (x)                   _
          /            R \        _ _ _ _  | |
  Py ---->        Time:1s  --->  |_|_|_|_| |_|
Time:10s                                   S2, Time:7s

The performance metrics have to be calculated are:
a. Mean interarrival time for Px and Py.
b. Mean service time for R (Router).
c. Mean service time for S1 (Server 1)
d. Mean service time for S2 (Server 2)
e. Total Px served by S1, total Py served by S2.

The simulation program runs until (200 s simulation time OR
Px served equal 100 packets).

Program created by:
1. Mohd Azi Bin Abdullah
2. Yahya Sjahrony

Adapted from the following resources:
1. Lecturer slide 2: Simulation Concepts and Components.
2. Lecturer slide 3: Inside Simulation Software.
3. Discrete-Event System Simulation - Jerry Banks (Chapter 4).
4. Event-Driven Simulation example program from Apache
   C++ Standard Library User's Guide.
5. Single-server queueing system, C++ version of mm1.c in
   Law--Kelton, 2000. Oct 2002.
6. CSC 270 simulation example, adapted March 1996 by J. Clarke from
   Turing original by M. Molle.

Sample command line:
./SimComplex -h

=========================================================
    Complex Network System Simulation         _ 
                                    _ _ _ _  | | 
    Px ---->                 --->  |_|_|_|_| |_| 
  Time:5s   \  _ _ _ _     /                 S1, Time:4s 
              |_|_|_|_| (x)                   _ 
            /            R \        _ _ _ _  | | 
    Py ---->        Time:1s  --->  |_|_|_|_| |_| 
  Time:10s                                   S2, Time:7s
=========================================================

Usage: ./SimComplex [options]
Options:
	-n : do not use random number stream generation
	-t : simulation ending time (default 200 s)
	-x : simulation ending packet count
	-d : increase debugging verbosity (-dd even more)
	-h : show this help and exit

Tested and compiled on:
1. Debian Wheezy with g++ (Debian 4.7.2-5) 4.7.2
2. C/C++ CodeBlocks IDE with Minimalist GNU compiler (MINGW) engine 
with gcc 4.7.1 Windows/unicode 32 bit.

Example outputs are in output directory.

