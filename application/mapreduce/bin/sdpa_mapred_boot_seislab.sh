#!/bin/sh
#sdpa boot -m 40*2**30 -s 1024*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2 MAP:1 PAR:2 RED:6
#sdpa boot -m 40*2**30 -s 2048*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2,2048*2**20 MAP:2 SHFL:6 RED:4 
#sdpa boot -m 32*2**30 -s 512*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2,256*2**20 MAP:3 SHFL:7 RED:2
#sdpa boot -m 32*2**30 -s 1024*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2,1024*2**20 MAP:4 RED:8
#sdpa boot -G 2 -m 32*2**30 -s 1024*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2,1024*2**20 MAP:4 RED:8
#sdpa boot -m 32*2**30 -s 1024*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2,1024*2**20 MAP:6 RED:12
#sdpa boot  -m 10*2**30 -s 1024*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2,1024*2**20 MAP:1 MAP+RED:11


#sdpa boot -m 32*2**30 -s 2*2**30 INI:1x1,0 FIN:1x1,0 LOAD:2,1024*2**20 MAP:2 MAP+RED:8 RED:2
#sdpa boot -m 32*2**30 -s 2*2**30 INI#-1:1x1,0 FIN#-1:1x1,0 LOAD#-1:2,1024*2**20 MAP+RED#-1:12
sdpa boot -m 32*2**30 -s 2*2**30 INI#-1:1x1,0 FIN#-1:1x1,0 LOAD#-1:4,1024*2**20 MAP#-1:4 RED#-1:8
