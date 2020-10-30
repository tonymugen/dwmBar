#ifndef config_hpp
#define config_hpp

#include <string>
#include <vector>

/** \brief Top delimiter
 *
 * Delimiter that goes between modules on the top bar
 */
static const std::string topDelimiter(" ");

/** \brief Bottom delimiter
 *
 * Delimiter that goes between modules on the bottom bar
 */
static const std::string bottomDelimiter(" | ");

/** \brief Are there two bars?
 *
 * If there is only one bar, the top bar information is used regardless of where you put your bar.
 */
static const bool twoBars = true;

/** \brief Delimiter between top and bottom
 *
 * The delimiter used by the dwm-extrabar patch.
 */
static const std::string botTopDelimiter(";");

/** List of top modules
 *
 * Names of modules for the top bar.
 * The necessary module information is:
 * - module name (one of the provided internal objects, the path to the relevant script, or a shell command)
 * - internal/external keyword
 * - refresh interval (in seconds; 0 means update only on receiving a real-time signal)
 * - `SIGRTMIN` signal ID, must be between 0 and 30.
 *   If the refresh interval is not zero, a real-time signal ca still be used to trigger the module before the interval expires.
 */
static const std::vector< std::vector<std::string> > topModuleList = {
	{"~/.scripts/checkMail",    "external", "0",   "8"},
	{"~/.scripts/pacupdate",    "external", "300", "9"},
	{"~/.scripts/getMicVolume", "external", "10",  "12"},
	{"~/.scripts/getVolume",    "external", "10",  "10"},
	{"~/.scripts/wifiSignal",   "external", "10",  "11"},
};

/** List of bottom modules
 *
 * Names of modules for the bottom bar.
 * See the top bar info for instructions.
 */
static const std::vector< std::vector<std::string> > bottomModuleList = {
	{"ModuleDate",          "internal", "60",  "1"},
	{"ModuleBattery",       "internal", "5",   "2"},
	{"ModuleCPU",           "internal", "2",   "3"},
	{"~/.scripts/gpuStats", "external", "10",   "4"},
	{"ModuleRAM",           "internal", "2",   "5"},
	{"ModuleDisk",          "internal", "10",  "6"},
	{"~/.scripts/wanIP",    "external", "300", "7"},
};

/** \brief Date format for the internal date/time module */
static const std::string dateFormat("%a %b %e %H:%M %Z");

/** \brief List of file systems to monitor
 *
 * File systems to monitor for available space using the built-in disk space module.
 */
static const std::vector<std::string> fsNames{"/home", "/home/tonyg/extra"};

#endif // config_hpp

