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

using std::string;
using std::vector;
using std::thread;
using std::this_thread::sleep_for;
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::chrono::seconds;

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
	//const string dateFormat("%a %b %e %R %Z");
	const string dateFormat("%a %b %e %H:%M %Z");
	vector<string> moduleOutputs(5);
	const string delim(" | ");
	vector<string> fsNames{"/home", "/home/tonyg/extra"};
	string barText;
	mutex mtx;
	condition_variable commonCond; // this triggers printing to the bar from individual modules
	vector<thread> moduleThreads;
	moduleThreads.push_back(thread{ModuleDate(60, dateFormat, &moduleOutputs[0], &commonCond, &signalCondition[1])});
	moduleThreads.push_back(thread{ModuleBattery(5, &moduleOutputs[1], &commonCond, &signalCondition[2])});
	moduleThreads.push_back(thread{ModuleCPU(2, &moduleOutputs[2], &commonCond, &signalCondition[3])});
	moduleThreads.push_back(thread{ModuleRAM(2, &moduleOutputs[3], &commonCond, &signalCondition[4])});
	moduleThreads.push_back(thread{ModuleDisk(30, fsNames, &moduleOutputs[4], &commonCond, &signalCondition[5])});
	while (true) {
		unique_lock<mutex> lk(mtx);
		commonCond.wait(lk);
		makeBarOutput(moduleOutputs, delim, barText);
		lk.unlock();
		std::cout << barText << " \n";
	}
	for (auto &t : moduleThreads){
		if ( t.joinable() ) {
			t.join();
		}
	}
	exit(0);
}

