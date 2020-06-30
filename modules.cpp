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
#include <functional>
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
using std::stod;
using std::stringstream;
using std::fstream;
using std::ios;
using std::time;
using std::put_time;
using std::localtime;
using std::this_thread::sleep_for;
using std::mutex;
using std::unique_lock;
using std::chrono::seconds;

using namespace DWMBspace;

void ModuleDate::operator()() const {
	if (refreshInterval_) { // if not zero, do a time-lapse loop
		while (true) {
			time_t t  = time(nullptr);
			stringstream outTime;
			outTime << put_time( localtime(&t), dateFormat_.c_str() );
			mutex mtx;
			unique_lock<mutex> lk(mtx);
			*outString_ = outTime.str();
			outputCondition_->notify_one();
			lk.unlock();
			outTime.clear();
			sleep_for( seconds(refreshInterval_) );
		}
	} else { // wait for a real-time signal
		mutex mtx;
		while (true) {
			unique_lock<mutex> lk(mtx);
			signalCondition_->wait(lk);
			time_t t  = time(nullptr);
			stringstream outTime;
			outTime << put_time( localtime(&t), dateFormat_.c_str() );
			*outString_ = outTime.str();
			outTime.clear();
			outputCondition_->notify_one();
			lk.unlock();
		}
	}
}

void ModuleBattery::operator()() const {
	if (refreshInterval_) { // if not zero, do a time-lapse loop
		while (true) {
			formatStatus_();
			sleep_for( seconds(refreshInterval_) );
		}
	} else {
		mutex mtxT;
		while (true) {
			unique_lock<mutex> lkT(mtxT);
			signalCondition_->wait(lkT);
			formatStatus_();
			lkT.unlock();
		}
	}
}

void ModuleBattery::formatStatus_() const {
	fstream statusStream;
	statusStream.open("/sys/class/power_supply/BAT0/status", ios::in);
	if ( !statusStream.is_open() ) { // fail silently
		return;
	}
	string batStatus;
	getline(statusStream, batStatus);
	statusStream.close();
	fstream capacityStream;
	capacityStream.open("/sys/class/power_supply/BAT0/capacity", ios::in);
	if ( !capacityStream.is_open() ) { // fail silently
		return;
	}
	string batCapacityStr;
	getline(capacityStream, batCapacityStr);
	capacityStream.close();
	double batCapacity = stod(batCapacityStr);
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

