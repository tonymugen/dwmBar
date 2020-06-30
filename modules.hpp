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

/// C++ modules for the status bar (definitions)
/** \file
 * \author Anthony J. Greenberg
 * \copyright Copyright (c) 2020 Anthony J. Greenberg
 * \version 0.9
 *
 *  Definitions of classes that provide output useful for display in the status bar.
 *
 */
#ifndef modules_hpp
#define modules_hpp

#include <bits/stdint-uintn.h>
#include <string>
#include <mutex>
#include <condition_variable>

using std::string;
using std::condition_variable;
using std::mutex;

namespace DWMBspace {

	/** \brief Base module class
	 *
	 * Establishes the common parameters for all modules. Modules are functors that write output to a `string` variable.
	 *
	 */
	class Module {
	public:
		/** \brief Destructor */
		virtual ~Module(){ outString_ = nullptr; outputCondition_ = nullptr; };
		/** Run the module
		 *
		 * Runs the module, refreshing at the specified interval or after receiving a refresh signal.
		 */
		virtual void operator()() const = 0;
	protected:
		/** Default constructor */
		Module() : refreshInterval_{0}, outString_{nullptr}, outputCondition_{nullptr}, signalCondition_{nullptr} {};
		/** Constructor
		 *
		 * \param[in] interval refresh time interval in seconds
		 * \param[in,out] output pointer to the output storing string
		 * \param[in,out] cVar pointer to the condition variable for change signaling
		 * \param[in,out] sigVar pointer to the condition variable to monitor real-time signals
		 */
		Module(const uint32_t &interval, string *output, condition_variable *cVar, condition_variable *sigVar) : refreshInterval_{interval}, outString_{output}, outputCondition_{cVar}, signalCondition_{sigVar} {};
		/** Refresh interval in seconds */
		uint32_t refreshInterval_;
		/** Pointer to the `string` that receives output */
		string *outString_;
		/** \brief Pointer to a condition variable to signal change in state
		 *
		 * The module is using this to communicate to the main thread.
		 */
		condition_variable *outputCondition_;
		/** \brief Pointer to a condition variable to accept signal events
		 *
		 * The module is waiting for this if it relies on a real-time signal to refresh.
		 */
		condition_variable *signalCondition_;
	};
	/** \brief Time and date derived class */
	class ModuleDate final : public Module {
	public:
		/** Default constructor */
		ModuleDate() : Module() {};
		/** Constructor
		 *
		 * \param[in] interval refresh time interval in seconds
		 * \param[in,out] output pointer to the output storing string
		 * \param[in,out] cVar pointer to the condition variable for change signaling
		 * \param[in,out] sigVar pointer to the condition variable to monitor real-time signals
		 */
		ModuleDate(const uint32_t &interval, const string &dateFormat, string *output, condition_variable *cVar, condition_variable *sigVar) : Module(interval, output, cVar, sigVar), dateFormat_{dateFormat} {};

		/** \brief Destructor */
		~ModuleDate() {};

		/** Run the module
		 *
		 * Runs the module, refreshing at the specified interval or after receiving a refresh signal.
		 */
		void operator()() const override;
	protected:
		/** \brief Time format string
		 *
		 * Date display format, same as for the Unix `date` command.
		 */
		string dateFormat_;
	};
	/** \brief Battery state derived class
	 *
	 * Displays the battery state.
	 */
	class ModuleBattery final : public Module {
	public:
		/** \brief Default constructor */
		ModuleBattery() : Module() {};
		/** Constructor
		 *
		 * \param[in] interval refresh time interval in seconds
		 * \param[in,out] output pointer to the output storing string
		 * \param[in,out] cVar pointer to the condition variable for change signaling
		 * \param[in,out] sigVar pointer to the condition variable to monitor real-time signals
		 */
		ModuleBattery(const uint32_t &interval, string *output, condition_variable *cVar, condition_variable *sigVar) : Module(interval, output, cVar, sigVar) {};
		/** \brief Destructor */
		~ModuleBattery() {};
		/** Run the module
		 *
		 * Runs the module, refreshing at the specified interval or after receiving a refresh signal.
		 */
		void operator()() const override;
	protected:
		/** \brief Get battery status and format output */
		void formatStatus_() const;
	};
}

#endif // modules_hpp
