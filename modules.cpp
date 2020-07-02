/*
 * Copyright (c) 2020 Anthony J. Greenberg
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/// C++ modules for the status bar (implementation)
/** \file
 * \author Anthony J. Greenberg
 * \copyright Copyright (c) 2020 Anthony J. Greenberg
 * \version 0.9
 *
 *  Implementation of classes that provide output useful for display in the status bar.
 *
 */
#include <cstddef>
#include <cstdio>
#include <sys/statvfs.h>
#include <ios>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "modules.hpp"

using std::string;
using std::stof;
using std::stoi;
using std::to_string;
using std::stringstream;
using std::fstream;
using std::ios;
using std::setprecision;
using std::fixed;
using std::time;
using std::put_time;
using std::localtime;
using std::this_thread::sleep_for;
using std::mutex;
using std::unique_lock;
using std::chrono::seconds;

using namespace DWMBspace;

void Module::operator()() const {
	if (refreshInterval_) { // if not zero, do a time-lapse loop
		while (true) {
			runModule_();
			sleep_for( seconds(refreshInterval_) );
		}
	} else { // wait for a real-time signal
		runModule_();
		mutex mtx;
		while (true) {
			unique_lock<mutex> lk(mtx);
			signalCondition_->wait(lk);
			runModule_();
			lk.unlock();
		}
	}
}

void ModuleDate::runModule_() const {
	time_t t = time(nullptr);
	stringstream outTime;
	outTime << put_time( localtime(&t), dateFormat_.c_str() );
	mutex mtx;
	unique_lock<mutex> lk(mtx);
	*outString_ = outTime.str();
	outputCondition_->notify_one();
	lk.unlock();
}

void ModuleBattery::runModule_() const {
	string batStatus;
	fstream statusStream;
	statusStream.open("/sys/class/power_supply/BAT0/status", ios::in);
	if ( statusStream.is_open() ) { // fail silently
		getline(statusStream, batStatus);
	}
	statusStream.close();
	string batCapacityStr;
	fstream capacityStream;
	capacityStream.open("/sys/class/power_supply/BAT0/capacity", ios::in);
	if ( capacityStream.is_open() ) { // fail silently
		getline(capacityStream, batCapacityStr);
	}
	capacityStream.close();
	float batCapacity = 0.0;
	if ( batCapacityStr.size() ) {
		batCapacity = stof(batCapacityStr);
	}
	mutex mtx;
	unique_lock<mutex> lk(mtx);
	if (batStatus == "Charging") {
		if (batCapacity < 5.0) {
			*outString_ = batCapacityStr + "% \uf58d";
		} else if (batCapacity < 20.0) {
			*outString_ = batCapacityStr + "% \uf585";
		} else if (batCapacity < 30.0) {
			*outString_ = batCapacityStr + "% \uf586";
		} else if (batCapacity < 40.0) {
			*outString_ = batCapacityStr + "% \uf587";
		} else if (batCapacity < 60.0) {
			*outString_ = batCapacityStr + "% \uf588";
		} else if (batCapacity < 80.0) {
			*outString_ = batCapacityStr + "% \uf589";
		} else if (batCapacity < 90.0) {
			*outString_ = batCapacityStr + "% \uf58a";
		} else if (batCapacity < 100.0){
			*outString_ = batCapacityStr + "% \uf578";
		}
	} else {
		if (batCapacity < 5.0) {
			*outString_ = batCapacityStr + "% \uf58d";
		} else if (batCapacity < 10.0) {
			*outString_ = batCapacityStr + "% \uf579";
		} else if (batCapacity < 20.0) {
			*outString_ = batCapacityStr + "% \uf57a";
		} else if (batCapacity < 30.0) {
			*outString_ = batCapacityStr + "% \uf57b";
		} else if (batCapacity < 40.0) {
			*outString_ = batCapacityStr + "% \uf57c";
		} else if (batCapacity < 50.0) {
			*outString_ = batCapacityStr + "% \uf57d";
		} else if (batCapacity < 60.0) {
			*outString_ = batCapacityStr + "% \uf57e";
		} else if (batCapacity < 70.0) {
			*outString_ = batCapacityStr + "% \uf57f";
		} else if (batCapacity < 80.0) {
			*outString_ = batCapacityStr + "% \uf580";
		} else if (batCapacity < 90.0) {
			*outString_ = batCapacityStr + "% \uf581";
		} else if (batCapacity < 100.0){
			*outString_ = batCapacityStr + "% \uf578";
		} else {
			if (batStatus == "Discharging") {
				*outString_ = batCapacityStr + "% \uf578";
			} else {
				*outString_ = batCapacityStr + "% \uf583";
			}
		}

	}
	outputCondition_->notify_one();
	lk.unlock();
}

void ModuleCPU::runModule_() const{
	string tempStatus;
	fstream tempStream;
	tempStream.open("/sys/class/thermal/thermal_zone0/temp", ios::in);
	if ( tempStream.is_open() ) {        // fail silently
		getline(tempStream, tempStatus);
	}
	tempStream.close();
	int32_t cpuTemp = 0;
	if ( tempStatus.size() ) {
		cpuTemp = stoi(tempStatus)/1000;
	}
	string loadLine;
	fstream loadFileStream;
	// the CPU usage data in this file are cumulative, so I must keep the values from the previous iteration (previous*_ private members)
	// I then subtract these previous values to get the data for the measurement interval
	loadFileStream.open("/proc/stat", ios::in);
	if ( loadFileStream.is_open() ) {    // fail silently
		getline(loadFileStream, loadLine);
	}
	loadFileStream.close();
	float curTotalLoad = 0.0;
	float curIdleLoad  = 0.0;
	float percentLoad  = 0.0;
	if ( loadLine.size() ) {
		string field;
		stringstream lineStream(loadLine);
		lineStream >> field; // first filed is the line name
		uint16_t fInd = 1;
		while (lineStream >> field) {
			if ( (fInd == 4) || (fInd == 5) ) {
				curIdleLoad  += stod(field);
				curTotalLoad += stod(field);
			} else {
				curTotalLoad += stod(field);
			}
			fInd++;
		}
		percentLoad        = ( 1.0 - (curIdleLoad - previousIdleLoad_)/(curTotalLoad - previousTotalLoad_) )*100;
		previousIdleLoad_  = curIdleLoad;
		previousTotalLoad_ = curTotalLoad;
	}
	string thermGlyph;
	if (cpuTemp < 35) {
		thermGlyph = "\ue20c";
	} else if (cpuTemp < 80) {
		thermGlyph = "\ue20a";
	} else {
		thermGlyph = "\ue20b";
	}
	stringstream pctStr;
	pctStr << fixed << setprecision(1) << percentLoad;
	const string loadOut = "\ufb19 " + pctStr.str() + "% " + thermGlyph + " " + to_string(cpuTemp) + "°C";
	mutex mtx;
	unique_lock<mutex> lk(mtx);
	*outString_ = loadOut;
	outputCondition_->notify_one();
	lk.unlock();
}

void ModuleRAM::runModule_() const {
	string memLine;
	fstream memInfoStream;
	memInfoStream.open("/proc/meminfo", ios::in);
	while ( getline(memInfoStream, memLine) ){
		if (memLine.compare(0, 8, "MemFree:") == 0) {
			break;
		}
	}
	memInfoStream.close();
	stringstream memLineStream(memLine);
	string freeMemStr;
	memLineStream >> freeMemStr;
	memLineStream >> freeMemStr;
	float memGi = stof(freeMemStr)/1048576.0; // the value in the file is in kb
	stringstream outMemStr;
	outMemStr << fixed << setprecision(1) << memGi;
	mutex mtx;
	unique_lock<mutex> lk(mtx);
	*outString_ = "\uf85a " + outMemStr.str() + "Gi";
	outputCondition_->notify_one();
	lk.unlock();
}

void ModuleDisk::runModule_() const {
	// start the output with the home icon for the home file system
	// (assuming that it's in the first element of the file system vector)
	string output;
	uint16_t iconInd = 0;
	for (auto &fs : fsNames_){
		output += (iconInd == 0 ? "\uf015 " : "  \uf0a0 ");
		iconInd++;
		struct statvfs buf;
		int test = statvfs(fs.c_str(), &buf);
		float diskSpace = 0.0;
		if (test == 0) {
			diskSpace = static_cast<float>(buf.f_bavail * buf.f_bsize)/1073741824.0;
		}
		stringstream dsStream;
		dsStream << fixed << setprecision(0) << diskSpace;
		output += dsStream.str() + "Gi";
		mutex mtx;
		unique_lock<mutex> lk(mtx);
		*outString_ = output;
		outputCondition_->notify_one();
		lk.unlock();
	}
}

