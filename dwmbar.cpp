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
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "modules.hpp"

using std::string;
using std::thread;
using std::this_thread::sleep_for;
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::chrono::seconds;

using namespace DWMBspace;

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
		std::cerr << "dmwbar ERROR: cannot open display\n";
		exit(1);
	}
	const int32_t screen = DefaultScreen(d);
	const Window root    = RootWindow(d, screen);
	XStoreName( d, root, barOutput.c_str() );
	XCloseDisplay(d);
}

void processSignal(int sig){
	std::cout << "got signal " << sig << "\n";
	exit(sig);	
}
int main(){
	//const string dateFormat("%a %b %e %R %Z");
	const int sigID = SIGRTMIN+1;
	signal(sigID, processSignal);
	const string dateFormat("%a %b %e %H:%M:%S %Z");
	string bar;
	string oldBar;
	mutex mtx;
	condition_variable dateCond;
	thread tstThr{ModuleDate(5, 12, dateFormat, &bar, &dateCond)};
	while (true) {
		unique_lock<mutex> lk(mtx);
		//dateCond.wait(lk, [&]{return oldBar != bar;});
		dateCond.wait(lk);
		if (oldBar != bar) {
			oldBar = bar;
		}
		lk.unlock();
		std::cout << oldBar << "\n";
	}
	tstThr.join();
	exit(0);
}

