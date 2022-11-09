/*
   Copyright (c) 2009-2020, Intel Corporation
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!     \file pcm-raw.cpp
  \brief Example of using CPU counters: implements a performance counter monitoring utility with raw events interface
  */
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h> // for gettimeofday()
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <assert.h>
#include <bitset>
#include "imc.h"

#include <vector>
#define PCM_DELAY_DEFAULT 1.0 // in seconds
#define PCM_DELAY_MIN 0.015 // 15 milliseconds is practical on most modern CPUs
#define MAX_CORES 4096
using namespace sprimc;
using namespace std;
//using namespace pcm;

bool show_partial_core_output = false;
bitset<MAX_CORES> ycores;
bool flushLine = false;

void print_usage(const string progname)
{
    cerr << "\n Usage: \n " << progname
         << " --help | [delay] [options] [-- external_program [external_program_options]]\n";
    cerr << "   <delay>                               => time interval to sample performance counters.\n";
    cerr << "                                            If not specified, or 0, with external program given\n";
    cerr << "                                            will read counters only after external program finishes\n";
    cerr << " Supported <options> are: \n";
    cerr << "  -h    | --help      | /h               => print this help and exit\n";
    cerr << "  -csv[=file.csv]     | /csv[=file.csv]  => output compact CSV format to screen or\n"
         << "                                            to a file, in case filename is provided\n";
    cerr << "  [-e event1] [-e event2] [-e event3] .. => list of custom events to monitor\n";
    cerr << "  event description example: -e core/config=0x30203,name=LD_BLOCKS.STORE_FORWARD/ -e core/fixed,config=0x333/ \n";
    cerr << "                             -e cha/config=0,name=UNC_CHA_CLOCKTICKS/ -e imc/fixed,name=DRAM_CLOCKS/\n";
    cerr << "  -yc   | --yescores  | /yc              => enable specific cores to output\n";
    cerr << "  -f    | /f                             => enforce flushing each line for interactive output\n";
    cerr << "  -i[=number] | /i[=number]              => allow to determine number of iterations\n";
    //print_help_force_rtm_abort_mode(41);
    cerr << " Examples:\n";
    cerr << "  " << progname << " 1                   => print counters every second without core and socket output\n";
    cerr << "  " << progname << " 0.5 -csv=test.log   => twice a second save counter values to test.log in CSV format\n";
    cerr << "  " << progname << " /csv 5 2>/dev/null  => one sampe every 5 seconds, and discard all diagnostic output\n";
    cerr << "\n";
}

uint64 getMCCounter(int ctr, ServerUncoreCounterState & before, ServerUncoreCounterState & after)
{
    uint64 sum = 0;
    for (uint32 ch =0; ch < 8; ++ch)
    {
        sum += after.MCCounter[ch][ctr]- before.MCCounter[ch][ctr];
    }
  
    return sum;
}

RawPMUConfigs allPMUConfigs;

void printfreecounters(vector<ServerUncoreCounterState>& BeforeUncoreState, vector<ServerUncoreCounterState>& AfterUncoreState, const CsvOutputType outputType)
{

    double cache[2][2];

    for (uint32 s = 0; s < 2; ++s)
    {
        cache[0][s] = 0;
        for (uint32 ch = 0; ch < 8; ++ch)
        {
            uint32 delta = AfterUncoreState[s].DRAMreads[s] -  BeforeUncoreState[s].DRAMreads[s];
            cache[0][s] += delta;
        }
    }

    for (uint32 s = 0; s < 2; ++s)
    {

        cache[1][s] = 0;
        for (uint32 ch = 0; ch < 8; ++ch)
        {
            uint32 delta = AfterUncoreState[s].DRAMwrites[s] -  BeforeUncoreState[s].DRAMwrites[s];
            cache[1][s] += delta; 
        }
    }

    cout << "W/R ratio: ";

    for (uint32 s = 0; s < 2; ++s){
        cout << cache[1][s] / cache[0][s] << ", ";
    }
    cout << endl;

}


void print(vector<ServerUncoreCounterState>& BeforeUncoreState, vector<ServerUncoreCounterState>& AfterUncoreState, const CsvOutputType outputType, int delay)
{
    cout << "W/R:" ;
    double ddrcyclecount =1e9 * (delay*60) / (1/2.4);
    for (uint32 s = 0; s < 2; ++s)
    {
        double write =  getMCCounter(0, BeforeUncoreState[s], AfterUncoreState[s]);
        double read =  getMCCounter(1, BeforeUncoreState[s], AfterUncoreState[s]);
        double wpq = getMCCounter(2, BeforeUncoreState[s], AfterUncoreState[s]);
        double rpq = getMCCounter(3, BeforeUncoreState[s], AfterUncoreState[s]);

        cout << write/read << ","<<"wpq="<<wpq/ddrcyclecount<<","<<"rpq="<<rpq/ddrcyclecount<<",";
    }

    cout << endl;
}

bool addEvent(string eventStr, pmc::IMC & imc)
{

    const enum pmuNameEnum {
        IMC,
        CHA
    };
    const std::map<std::string, pmuNameEnum> pmuNameMap{{"imc", IMC},{"cha", CHA}};

    RawEventConfig config = { {0,0,0}, "" };
    const auto typeConfig = split(eventStr, '/');
    if (typeConfig.size() < 2)
    {
        cerr << "ERROR: wrong syntax in event description \"" << eventStr << "\"\n";
        return false;
    }
    auto pmuName = typeConfig[0];
    if (pmuName.empty())
    {
        pmuName = "core";
        // TODO:: what do we do here?
    }
    const auto configStr = typeConfig[1];
    if (configStr.empty())
    {
        cerr << "ERROR: empty config description in event description \"" << eventStr << "\"\n";
        return false;
    }

    switch(pmuNameMap[pmuName]){
        case IMC:
            imc.program(configStr);
        case CHA:
        default:
            return false;
    }

    const auto configArray = split(configStr, ',');
    bool fixed = false;
    for (auto item : configArray)
    {
        string  f0, f1, f2;
        //if (match(item, "config=", &config.first[0])) {}
        cout << "Item is " << item << "\n";
        if (match("config=(0[xX][0-9a-fA-F]+)", item, f0)) {
        config.first[0] = strtoll(f0.c_str(), NULL, 16);
        cout << "Config read" << config.first[0] << "\n";	
        }
        else if (match("config1=(0[xX][0-9a-fA-F]+)", item, f1)) {
        config.first[1] = strtoll(f1.c_str(), NULL, 16);
        cout  << "Config1 read" << config.first[1] << "\n";	
        }
        else if (match("config2=(0[xX][0-9a-fA-F]+)", item, f2)) {
        config.first[2] = strtoll(f2.c_str(), NULL, 16);
        cout  << "Config2 read" << config.first[2] << "\n";	
        }
        else if (match("name=(.*)", item, config.second)) {}
        //else if (pcm_sscanf(item) >> s_expect("name=") >> setw(255) >> config.second) {}
        else if (item == "fixed")
        {
            fixed = true;
        }
        else
        {
            cerr << "ERROR: unknown token " << item << " in event description \"" << eventStr << "\"\n";
            return false;
        }
    }
    cout << "parsed "<< (fixed?"fixed ":"")<<"event " << pmuName << ": \"" << hex << config.second << "\" : {0x" << hex << config.first[0] << ", 0x" << config.first[1] << ", 0x" << config.first[2] << "}\n" << dec;
    if (fixed)
        allPMUConfigs[pmuName].fixed.push_back(config);
    else
        allPMUConfigs[pmuName].programmable.push_back(config);
    return true;
}




int main(int argc, char* argv[])
{
    //set_signal_handlers();


    cerr << "\n";
    cerr << " Processor Counter Monitor: Raw Event Monitoring Utility \n";
    cerr << "\n";

    double delay = -1.0;
    string program = string(argv[0]);
    int iteration = 1;

    pcm::IMC imc;

    if (argc > 1) do
    {
        argv++;
        argc--;
        if (strncmp(*argv, "--help", 6) == 0 ||
            strncmp(*argv, "-h", 2) == 0 ||
            strncmp(*argv, "/h", 2) == 0)
        {
            print_usage(program);
            exit(EXIT_FAILURE);
        }

        else if (strncmp(*argv, "-i", 2) == 0)
        {
            argv++;
            argc--;
            iteration = atoi(*argv);
            cout << "Iteration = " << iteration << "\n";
        }

        else if (strncmp(*argv, "-e", 2) == 0)
        {
            argv++;
            argc--;

            if (addEvent(*argv, imc) == false)
            {
                exit(EXIT_FAILURE);
            }

            continue;
        }
        else if (strncmp(*argv, "-d", 2) == 0)
        {
            argv++;
            argc--;
            delay = atof(*argv);
            cout << "Delay in seconds " << delay  << "\n";

            // any other options positional that is a floating point number is treated as <delay>,
            // while the other options are ignored with a warning issues to stderr
        }
    } while (argc > 1); // end of command line parsing loop


    if (delay <= 0.0) delay = PCM_DELAY_DEFAULT;

    cerr << "Update every " << delay << " seconds\n";

    std::cout.precision(4);
    std::cout << std::fixed;

    std::vector<std::vector<uint64>> counter0, prev0;
    std::vector<std::vector<uint64>> counter1, prev1;
    std::vector<std::vector<uint64>> counter2, prev2;
    std::vector<std::vector<uint64>> counter3, prev3;

    imc.getMCCounter(prev0, 0);
    imc.getMCCounter(prev1, 1);
    imc.getMCCounter(prev2, 2);
    imc.getMCCounter(prev3, 3);

    double write, read, wpq, rpq;
    double ddrcyclecount = 1e9 * (delay*60) / (1/2.4);

    while (1){

        ::sleep(delay);

        imc.getMCCounter(counter0, 0);
        imc.getMCCounter(counter1, 1);
        imc.getMCCounter(counter2, 2);
        imc.getMCCounter(counter3, 3);

        for(int i = 0; i < 2; i++){

            write = 0;
            read = 0;
            wpq = 0;
            rpq = 0;

            for(int j = 0; j < counter0[i].size(); j++){
                write += counter0[i][j] - prev0[i][j];
                read  += counter1[i][j] - prev1[i][j];
                wpq   += counter2[i][j] - prev2[i][j];
                rpq   += counter3[i][j] - prev3[i][j];
            }

            std::cout << "W/R: " << write/read << ", wpq = " << wpq/ddrcyclecount << ", rpq = " << rpq/ddrcyclecount << std::endl;
        }


        prev0 = counter0;
        prev1 = counter1;
        prev2 = counter2;
        prev3 = counter3;
    }

}
