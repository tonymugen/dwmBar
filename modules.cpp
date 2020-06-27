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
#include <functional>
#include <string>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "modules.hpp"

using std::string;
using std::stringstream;
using std::time;
using std::put_time;
using std::localtime;
using std::this_thread::sleep_for;
using std::mutex;
using std::unique_lock;
using std::chrono::seconds;

using namespace DWMBspace;

void ModuleDate::operator()() const {
	if (refreshInterval_) { // if not zero, do a loop
		while (1) {
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
	} else { // for now, if the interval is 0, do only once. TODO: wait for signal
		time_t t  = time(nullptr);
		stringstream outTime;
		outTime << put_time( localtime(&t), dateFormat_.c_str() );
		*outString_ = outTime.str();
		outTime.clear();
	}
}

