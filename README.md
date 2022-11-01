# SPR_UncoreMon
#make executable
make

#current code is hardcded to use all 4 GP counters.
#counter 0=write.cas 1=read.cas 2=write.OCCupancy and 3-read.OCCupancy. following CMD should be used.
#value of d indicates no of  seconds set for  sampling period.

./IMC-raw.x -e imc/config=0x000000000000f005,name=UNC_M_CAS_COUNT.WR -e imc/config=0x000000000000cf05,name=UNC_M_CAS_COUNT.RD  -e imc/config=0x0000000000000082,name=UNC_M_WPQ_OCCUPANCY_PCH0 -e imc/config=0x0000000000000080,name=UNC_M_RPQ_OCCUPANCY_PCH0  -d 2






#get read and write CAS with below -i number of iterations -d sample time in seconds this has been only designed to support 1 second and higher sample time.
#delay could be modified to chnage for lower resolution
./IMC-raw.x -e imc/config=0x000000000000cf05,name="UNC_M_CAS_COUNT.RD" -e imc/config=0x000000000000f005,name="UNC_M_CAS_COUNT.WR" -i 4 -d 2
#config value for any other IMC counter could be obtained with emon
 emon --dry-run -t1 -l10 -C "UNC_M_CAS_COUNT.WR" -m

#Todo

Currently within each socket Uncore
two setsof MCC counters are defined eahc for one socket. This is redundant. Just defined 4x8 MCC counters for each socket.
