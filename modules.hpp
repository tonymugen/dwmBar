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

#include <cstddef>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

using std::vector;
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
		void operator()() const;
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
		/** \brief Run the module once
		 *
		 * Retrieves the data specific to the module and formats the output.
		 */
		virtual void runModule_() const = 0;
	};

	/** \brief Time and date */
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

	protected:
		/** \brief Time format string
		 *
		 * Date display format, same as for the Unix `date` command.
		 */
		string dateFormat_;
		/** \brief Run the module once
		 *
		 * Retrieves the data specific to the module and formats the output.
		 */
		void runModule_() const override;
	};

	/** \brief Battery state
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
	protected:
		/** \brief Run the module once
		 *
		 * Retrieves the data specific to the module and formats the output.
		 */
		void runModule_() const override;
	};

	/** \brief CPU status
	 *
	 * Displays CPU temperature and load.
	 *
	 */
	class ModuleCPU final : public Module {
	public:
		/** \brief Default constructor */
		ModuleCPU() : Module() {};
		/** Constructor
		 *
		 * \param[in] interval refresh time interval in seconds
		 * \param[in,out] output pointer to the output storing string
		 * \param[in,out] cVar pointer to the condition variable for change signaling
		 * \param[in,out] sigVar pointer to the condition variable to monitor real-time signals
		 */
		ModuleCPU(const uint32_t &interval, string *output, condition_variable *cVar, condition_variable *sigVar) : Module(interval, output, cVar, sigVar), previousTotalLoad_{0.0}, previousIdleLoad_{0.0} {};
		/** \brief Destructor */
		~ModuleCPU() {};
	protected:
		/** \brief Previous total CPU time */
		mutable float previousTotalLoad_;
		/** \brief Previous idle CPU time */
		mutable float previousIdleLoad_;
		/** \brief Run the module once
		 *
		 * Retrieves the data specific to the module and formats the output.
		 */
		void runModule_() const override;
	};
	/** \brief Free memory
	 *
	 * Displays the amount of free RAM.
	 */
	class ModuleRAM final : public Module {
	public:
		/** \brief Default constructor */
		ModuleRAM() : Module() {};
		/** Constructor
		 *
		 * \param[in] interval refresh time interval in seconds
		 * \param[in,out] output pointer to the output storing string
		 * \param[in,out] cVar pointer to the condition variable for change signaling
		 * \param[in,out] sigVar pointer to the condition variable to monitor real-time signals
		 */
		ModuleRAM(const uint32_t &interval, string *output, condition_variable *cVar, condition_variable *sigVar) : Module(interval, output, cVar, sigVar) {};
		/** \brief Destructor */
		~ModuleRAM() {};
	protected:
		/** \brief Run the module once
		 *
		 * Retrieves the data specific to the module and formats the output.
		 */
		void runModule_() const override;
	};
	/** \brief Disk free space
	 *
	 * Lists free space in a list of file systems in Gb.
	 */
	class ModuleDisk final : public Module {
	public:
		/** \brief Default constructor */
		ModuleDisk() : Module() {};
		/** Constructor
		 *
		 * \param[in] interval refresh time interval in seconds
		 * \param[in] fsVector vector of file system names
		 * \param[in,out] output pointer to the output storing string
		 * \param[in,out] cVar pointer to the condition variable for change signaling
		 * \param[in,out] sigVar pointer to the condition variable to monitor real-time signals
		 */
		ModuleDisk(const uint32_t &interval, const vector<string> &fsVector, string *output, condition_variable *cVar, condition_variable *sigVar) : Module(interval, output, cVar, sigVar), fsNames_{fsVector} {};
		/** \brief Destructor */
		~ModuleDisk() {};
	protected:
		/** \brief File system names */
		vector<string> fsNames_;
		/** \brief Run the module once
		 *
		 * Retrieves the data specific to the module and formats the output.
		 */
		void runModule_() const override;
	};
	/** \brief External scripts
	 *
	 * Runs an external script or shell command and displays the output.
	 * No formatting of the external output is performed, but it is truncated to 500 characters.
	 */
	class ModuleExtern final : public Module {
	public:
		/** \brief Default constructor */
		ModuleExtern() : Module() {};
		/** Constructor
		 *
		 * \param[in] interval refresh time interval in seconds
		 * \param[in] command external command
		 * \param[in,out] output pointer to the output storing string
		 * \param[in,out] cVar pointer to the condition variable for change signaling
		 * \param[in,out] sigVar pointer to the condition variable to monitor real-time signals
		 */
		ModuleExtern(const uint32_t &interval, const string &command, string *output, condition_variable *cVar, condition_variable *sigVar) : Module(interval, output, cVar, sigVar), extCommand_{command} {};
		/** \brief Destructor */
		~ModuleExtern() {};
	protected:
		/** \brief Output length limit */
		static const size_t lengthLimit_;
		/** \brief External command string */
		const string extCommand_;
		/** \brief Run the module once
		 *
		 * Runs the external shell command or script and returns the output, truncating to 500.
		 */
		void runModule_() const override;
	};
}

#endif // modules_hpp
