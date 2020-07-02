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
/// A bar for dwm
/** \file
 * \author Anthony J. Greenberg
 * \copyright Copyright (c) 2020 Anthony J. Greenberg
 * \version 0.9
 *
 * Displays information on the bar for the Dynamic Window Manager (dwm). External scripts and some internal functions are supported.
 * Can use two bars (bottom and top) if dwm is patched with `dwm-extrabar`.
 *
 */
#include <X11/Xlib.h>
#include <bits/stdint-intn.h>
#include <csignal>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "modules.hpp"
// modify this file to configure what modules go where
#include "config.hpp"

using std::string;
using std::stoi;
using std::vector;
using std::thread;
using std::this_thread::sleep_for;
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::chrono::seconds;
using std::cerr;

using namespace DWMBspace;

/** \brief Number of possible real-time signals */
static const int sigRTNUM = 30;
/** \brief Condition variables that will respond to real-time signals */
static vector<condition_variable> signalCondition(sigRTNUM);

/** \brief Make bar output
 *
 * Takes individual module outputs and puts them together for printing.
 *
 * \param[in] moduleOutput vector of individual module outputs
 * \param[in] delimiter delimiter character(s) between modules
 * \param[out] barText compiled text to be printed to the bar
 */
void makeBarOutput(const vector<string> &moduleOutput, const string &delimiter, string &barText){
	barText.clear();
	for (auto moIt = moduleOutput.begin(); moIt != (moduleOutput.end() - 1); ++moIt){
		barText += (*moIt) + delimiter;
	}
	barText += moduleOutput.back();
}

/** \brief Render the bar
 *
 * Renders the bar text by printing the provided string to the root window.
 * This is how dwm handles status bars.
 *
 * \param[in] barOutput text to be displayed
 */
void printRoot(const string &barOutput){
	Display *d = XOpenDisplay(NULL);
	if (d == nullptr) {
		return;         // fail silently
	}
	const int32_t screen = DefaultScreen(d);
	const Window root    = RootWindow(d, screen);
	XStoreName( d, root, barOutput.c_str() );
	XCloseDisplay(d);
}

/** \brief Process real-time signals
 *
 * Receive and process real-time signals to trigger relevant modules.
 *
 * \param[in] sig signal number (starting at `SIGRTMIN`)
 */
void processSignal(int sig){
	if ( (sig < SIGRTMIN) || (sig > SIGRTMAX) ) { // do nothing silently if wrong signal received
		return;
	}
	size_t sigInd = sig - SIGRTMIN;
	signalCondition[sigInd].notify_one();
}

int main(){
	for (int sigID = SIGRTMIN; sigID <= SIGRTMAX; sigID++) {
		signal(sigID, processSignal);
	}
	mutex mtx;
	condition_variable commonCond; // this triggers printing to the bar from individual modules
	vector<string> topModuleOutputs( topModuleList.size() );
	vector<thread> moduleThreads;
	size_t moduleID = 0;
	for (auto &tb : topModuleList){
		if (tb.size() != 4) {
			cerr << "ERROR: top bar module description vector must be have exactly four elements, yours has " << tb.size() << " (module " << tb[0] << ")\n";
			exit(1);
		}
		if (tb[1] == "external") {
			int32_t interval = stoi(tb[2]);
			if (interval < 0) {
				cerr << "ERROR: refresh interval cannot be negative, yours is " << interval << " (module " << tb[0] << ")\n";
				exit(2);
			}
			int32_t rtSig = stoi(tb[3]);
			if (rtSig < 0) {
				cerr << "ERROR: real-time signal cannot be negative, yours is " << rtSig << " (module " << tb[0] << ")\n";
				exit(3);
			}
			moduleThreads.push_back(thread{ModuleExtern(interval, tb[0], &topModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
		} else {
			int32_t interval = stoi(tb[2]);
			if (interval < 0) {
				cerr << "ERROR: refresh interval cannot be negative, yours is " << interval << " (module " << tb[0] << ")\n";
				exit(2);
			}
			int32_t rtSig = stoi(tb[3]);
			if (rtSig < 0) {
				cerr << "ERROR: real-time signal cannot be negative, yours is " << rtSig << " (module " << tb[0] << ")\n";
				exit(3);
			}
			if (tb[0] == "ModuleDate") {
				moduleThreads.push_back(thread{ModuleDate(interval, dateFormat, &topModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
			} else if (tb[0] == "ModuleBattery") {
				moduleThreads.push_back(thread{ModuleBattery(interval, &topModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
			} else if (tb[0] == "ModuleCPU") {
				moduleThreads.push_back(thread{ModuleCPU(interval, &topModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
			} else if (tb[0] == "ModuleRAM") {
				moduleThreads.push_back(thread{ModuleRAM(interval, &topModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
			} else if (tb[0] == "ModuleDisk") {
				moduleThreads.push_back(thread{ModuleDisk(interval, fsNames, &topModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
			} else {
				cerr << "ERROR: unknown internal module " << tb[0] << "\n";
				exit(4);
			}
		}
		moduleID++;
	}
	vector<string> bottomModuleOutputs;
	if (twoBars) {
		bottomModuleOutputs.resize( bottomModuleList.size() );
		moduleID = 0;
		for (auto &bb : bottomModuleList){
			if (bb.size() != 4) {
				cerr << "ERROR: top bar module description vector must be have exactly four elements, yours has " << bb.size() << " (module " << bb[0] << ")\n";
				exit(1);
			}
			if (bb[1] == "external") {
				int32_t interval = stoi(bb[2]);
				if (interval < 0) {
					cerr << "ERROR: refresh interval cannot be negative, yours is " << interval << " (module " << bb[0] << ")\n";
					exit(2);
				}
				int32_t rtSig = stoi(bb[3]);
				if (rtSig < 0) {
					cerr << "ERROR: real-time signal cannot be negative, yours is " << rtSig << " (module " << bb[0] << ")\n";
					exit(3);
				}
				moduleThreads.push_back(thread{ModuleExtern(interval, bb[0], &bottomModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
			} else {
				int32_t interval = stoi(bb[2]);
				if (interval < 0) {
					cerr << "ERROR: refresh interval cannot be negative, yours is " << interval << " (module " << bb[0] << ")\n";
					exit(2);
				}
				int32_t rtSig = stoi(bb[3]);
				if (rtSig < 0) {
					cerr << "ERROR: real-time signal cannot be negative, yours is " << rtSig << " (module " << bb[0] << ")\n";
					exit(3);
				}
				if (bb[0] == "ModuleDate") {
					moduleThreads.push_back(thread{ModuleDate(interval, dateFormat, &bottomModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
				} else if (bb[0] == "ModuleBattery") {
					moduleThreads.push_back(thread{ModuleBattery(interval, &bottomModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
				} else if (bb[0] == "ModuleCPU") {
					moduleThreads.push_back(thread{ModuleCPU(interval, &bottomModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
				} else if (bb[0] == "ModuleRAM") {
					moduleThreads.push_back(thread{ModuleRAM(interval, &bottomModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
				} else if (bb[0] == "ModuleDisk") {
					moduleThreads.push_back(thread{ModuleDisk(interval, fsNames, &bottomModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
				} else {
					cerr << "ERROR: unknown internal module " << bb[0] << "\n";
					exit(4);
				}
			}
			moduleID++;
	}
	}
	string barTextBottom;
	string barText;
	while (true) {
		unique_lock<mutex> lk(mtx);
		commonCond.wait(lk);
		makeBarOutput(topModuleOutputs, topDelimiter, barText);
		if (twoBars) {
			makeBarOutput(bottomModuleOutputs, bottomDelimiter, barTextBottom);
			// I personally like a little adding around the top bar. Change to suit your taste.
			barText = " " + barText + " " + botTopDelimiter + barTextBottom;
		}
		lk.unlock();
		printRoot(barText);
	}
	for (auto &t : moduleThreads){
		if ( t.joinable() ) {
			t.join();
		}
	}
	exit(0);
}

