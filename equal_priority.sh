source hwdrc_osmailbox_config.inc.sh



#HP_CORES provide as arg2 in ythe format 0-23
#LP_CORES provide as arg3 in ythe format 24-47

COREGRP1=$2
COREGRP2=$3

pqos -R

#g_CLOSToMEMCLOS for hwdrc_settings_update()
#Assume MCLOS0 is highest priority and MCLOS1-2 has lower priority accordingly, MCLOS 3 with lowest priority
#Map CLOS 0-3, 8-15 and CLOS4 to MCLOS 0(HP), CLOS 5-MCLOS 1, CLOS6- MCLOS 2, CLOS7-MCLOS3(LP)
#g_CLOSToMEMCLOS=0x3FFFE4FC
#Refer to CLOS to MEMCLOS
g_CLOSToMEMCLOS=0x0000E400

#MEM_CLOS_ATTRIBUTES 
#ALL MCLOS0-3 all Same priority MAX delay 0xff, MIN delay 0x1, priority 0xf as we are setting both MLCO

#using equal throttling for all  MCLOS0-MCLO3
g_ATTRIBUTES_MCLOS0=0x80ff010f
g_ATTRIBUTES_MCLOS1=0x81ff010f
g_ATTRIBUTES_MCLOS2=0x82ff010f
g_ATTRIBUTES_MCLOS3=0x83ff010f

#CONFIG0
#DRC Setpoint threshold passed as arg1
#enable MEM_CLOS_EVEMT
#0x06 used for noise filtering
g_CONFIG0=$((0x01050600 + $1))

#Here the OS_MAILBOX is per_socket, so we need to pick a core from the socket you want, one core msr settings will be enough to represent the socket setup
core_id=1
echo "init DRC to default settings for Scoket0; selected core_id in socket0 is $core_id"
hwdrc_settings_update
hwdrc_reg_dump


core_id=57
echo "init DRC to default settings for Scoket1; selected core_id in socket1 is $core_id"
hwdrc_settings_update
hwdrc_reg_dump

#Above CLOS:7 was attached to MEMCLOS3 and CLOS:4 was atached to MEMCLOS0
#So all cores cana be attched to the same MEMCLOS for equal priority. For example as shown below all cores is attached to to MEMCLOS0
pqos -a llc:4=$COREGRP1
pqos -a llc:4=$COREGRP2
