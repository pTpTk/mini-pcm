#
g_ret_data=0
g_ret_interface=0

wait_until_run_busy_cleared(){
run_busy=1
while [[ $run_busy -ne 0 ]]
do 
  rd_interface=`rdmsr -p $core_id 0xb0`
  run_busy=$[rd_interface & 0x80000000]
  if [ $run_busy -eq 0 ]; then
    #not busy, just return
    break
  else
    echo "====warning:RUN_BUSY=1.sleep 1,then retry"
    sleep 1
  fi
done
}

hwdrc_write(){
#input 1: the value of OS Mailbox Interface for write operation
#input 2: the value of OS Mailbox Data
#return OSmailbox interface status in g_ret_interface
value_interface=$1
value_data=$2

wait_until_run_busy_cleared
wrmsr -p $core_id 0xb1 $value_data
#the value_interface should include the RUN_BUSY,and all other fileds including COMMANDID,sub-COMMNADID,MCLOS ID(for attribute)
wrmsr -p $core_id 0xb0 $value_interface

wait_until_run_busy_cleared
g_ret_interface=`rdmsr -p $core_id 0xb0`
}

hwdrc_read(){
#input: the value of OS Mailbox Interface for read operation
#retrun hwdrc reg read value in $g_ret_data
#return OSmailbox interface status in $g_ret_interface
value_interface=$1

wait_until_run_busy_cleared
wrmsr -p $core_id 0xb0 $value_interface

wait_until_run_busy_cleared

g_ret_interface=`rdmsr -p $core_id 0xb0`
#g_ret_data=`rdmsr -p $core_id 0xb1`
g_ret_data=`rdmsr -p $core_id 0xb1 --zero-pad`
g_ret_data=${g_ret_data:8:8}
}


hwdrc_settings_update() {
#input 1: $g_CLOSToMEMCLOS
#input 2: $g_ATTRIBUTES_MCLOS0
#input 3: $g_ATTRIBUTES_MCLOS1
#input 4: $g_ATTRIBUTES_MCLOS2
#input 5: $g_ATTRIBUTES_MCLOS3
#input 6: $g_CONFIG0

#1.	Disable HWDRC: MEMCLOS_EN
#disable MEMCLOS DRC, then do the DRC settings update
#RD_WR=WR
#MEMCLOS_EN=0
echo "MEMCLOS_EN=0"
hwdrc_write 0x810054d0 0x0

#2.	Config CLOS to MEMCLOS mapping: CLOSToMEMCLOS
printf "CLOSToMEMCLOS=%x\n" $g_CLOSToMEMCLOS
hwdrc_write 0x810050d0 $g_CLOSToMEMCLOS

#3.	Setup MemCLOS0~3 in MEM_CLOS_ATTRIBUTES
hwdrc_write 0x810051d0 $g_ATTRIBUTES_MCLOS0
echo "MEM_CLOS_ATTRIBUTES: MCLOS0="$g_ATTRIBUTES_MCLOS0

hwdrc_write 0x810851d0 $g_ATTRIBUTES_MCLOS1
echo "MEM_CLOS_ATTRIBUTES: MCLOS1="$g_ATTRIBUTES_MCLOS1

hwdrc_write 0x811051d0 $g_ATTRIBUTES_MCLOS2 
echo "MEM_CLOS_ATTRIBUTES: MCLOS2="$g_ATTRIBUTES_MCLOS2

hwdrc_write 0x811851d0 $g_ATTRIBUTES_MCLOS3 
echo "MEM_CLOS_ATTRIBUTES: MCLOS3="$g_ATTRIBUTES_MCLOS3

#4.	config the DRC: CONFIG0
hwdrc_write 0x810052d0 $g_CONFIG0 
echo "CONFIG0=" $g_CONFIG0

#5.	enable the HWDRC: MEMCLOS_EN=1
#RD_WR=WR
#MEMCLOS_EN=1
hwdrc_write 0x810054d0 0x2 
echo "MEMCLOS_EN=0x2"

}

hwdrc_disable(){
#disable MEMCLOS DRC, then do the DRC settings update
#RD_WR=WR
#MEMCLOS_EN=0
echo "MEMCLOS_EN=0"
hwdrc_write 0x810054d0 0x0
}

hwdrc_enable(){

hwdrc_write 0x810054d0 0x2
echo "MEMCLOS_EN=0x2"

}


hwdrc_reg_dump(){
echo "dump all of the DRC registers:"
hwdrc_read 0x800054d0
echo "MEMCLOS_EN="$g_ret_data

hwdrc_read 0x800050d0
echo "CLOSToMEMCLOS="$g_ret_data

hwdrc_read 0x800051d0
echo "ATTRIBUTES:MLCOS0=" $g_ret_data

hwdrc_read 0x800851d0
echo "ATTRIBUTES:MLCOS1=" $g_ret_data

hwdrc_read 0x801051d0
echo "ATTRIBUTES:MLCOS2=" $g_ret_data

hwdrc_read 0x801851d0
echo "ATTRIBUTES:MLCOS3=" $g_ret_data

hwdrc_read 0x800052d0
echo "CONFIG0=" $g_ret_data

}

