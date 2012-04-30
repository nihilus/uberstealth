// Represents the settings of a single configuration profile.

#pragma once

#include <vector>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <RDTSCEmu/driver/RDTSCEmu.h>

namespace uberstealth {

enum InlinePatching { AutoSelect, ForceAbsolute };

class HideDebuggerProfile {
#define GENERAL_ACCESSORS(config_name, config_path, data_type, default_value) data_type get##config_name##config_path() const { return pt_.get(#config_name"."#config_path, default_value); } \
																			  void set##config_name##config_path(const data_type& value) { pt_.put(#config_name"."#config_path, value); }

// Same as GENERAL_ACCESSORS but also expects a translator so the corresponding type can be represented as std::string.
#define TRANSLATOR_ACCESSORS(config_name, config_path, data_type, default_value, translator) data_type get##config_name##config_path() const { return pt_.get(#config_name"."#config_path, default_value, translator##()); } \
																							 void set##config_name##config_path(data_type value) { pt_.put(#config_name"."#config_path, value, translator##()); }

#define ENABLED_ACCESSORS(config_name)									  GENERAL_ACCESSORS(config_name, Enabled, bool, false)
#define VALUE_ACCESSORS(config_name, data_type, default_value)			  GENERAL_ACCESSORS(config_name, Value, data_type, default_value)

private:
	// Custom property_tree translator for the InlinePatching enum.
	struct InlinePatchingTranslator	{
		InlinePatching get_value(const std::string& value) const {
			if (value == "AutoSelect") return AutoSelect;
			else if (value == "ForceAbsolute") return ForceAbsolute;
			else throw std::runtime_error("Unable to convert given string to InlinePatching enum (" + value + ")");
		}

		std::string put_value(InlinePatching value) const {
			switch (value) {
			case AutoSelect: return "AutoSelect";
			case ForceAbsolute: return "ForceAbsolute";
			default: throw std::runtime_error("Unable to convert unknown InlinePatching enum to string");
			}
		}
	};

	// Custom property_tree translator for the RDTSCMode enum.
	struct RDTSCModeTranslator {
		RDTSCMode get_value(const std::string& value) const	{
			if (value == "Constant") return constant;
			else if (value == "Increasing") return increasing;
			else throw std::runtime_error("Unable to convert given string to RDTSCMode enum (" + value + ")");
		}

		std::string put_value(RDTSCMode value) const {
			switch (value) {
			case constant: return "Constant";
			case increasing: return "Increasing";
			default: throw std::runtime_error("Unable to convert unknown RDTSCMode enum to string");
			}
		}
	};

public:
	HideDebuggerProfile(const boost::property_tree::ptree& pt) : pt_(pt) {}
	HideDebuggerProfile() {}

	ENABLED_ACCESSORS(BlockInput)
	ENABLED_ACCESSORS(DbgPrintException)
	ENABLED_ACCESSORS(EnableDbgAttach)
	ENABLED_ACCESSORS(EnableDbgStart)
	ENABLED_ACCESSORS(FakeParentProcess)
	ENABLED_ACCESSORS(GetTickCount)
	ENABLED_ACCESSORS(GetVersion)
	ENABLED_ACCESSORS(HaltAfterSEHHandler)
	ENABLED_ACCESSORS(HaltInSEHHandler)
	ENABLED_ACCESSORS(HeapFlags)
	ENABLED_ACCESSORS(HideDebuggerProcess)
	ENABLED_ACCESSORS(HideDebuggerWindows)
	ENABLED_ACCESSORS(KillAntiAttach)
	ENABLED_ACCESSORS(LoadStealthDriver)
	ENABLED_ACCESSORS(LogSEH)
	ENABLED_ACCESSORS(NtClose)
	ENABLED_ACCESSORS(NtGlobalFlag)
	ENABLED_ACCESSORS(NtQueryInformationProcess)
	ENABLED_ACCESSORS(NtQueryObject)
	ENABLED_ACCESSORS(NtQuerySystemInformation)
	ENABLED_ACCESSORS(NtSetInformationThread)
	ENABLED_ACCESSORS(NtTerminate)
	ENABLED_ACCESSORS(NtYieldExecution)
	ENABLED_ACCESSORS(OpenProcess)
	ENABLED_ACCESSORS(OutputDbgString)
	ENABLED_ACCESSORS(PassUnknownExceptions)
	ENABLED_ACCESSORS(PEBIsBeingDebugged)
	ENABLED_ACCESSORS(ProtectDebugRegisters)
	ENABLED_ACCESSORS(RtlGetNtGlobalFlags)
	
	ENABLED_ACCESSORS(SuspendThread)
	ENABLED_ACCESSORS(SwitchDesktop)
	ENABLED_ACCESSORS(UnloadStealthDriver)
	TRANSLATOR_ACCESSORS(InlinePatchingMethod, Value, InlinePatching, AutoSelect, InlinePatchingTranslator)
	
	TRANSLATOR_ACCESSORS(RDTSCDriver, Mode, RDTSCMode, constant, RDTSCModeTranslator)
	GENERAL_ACCESSORS(RDTSCDriver, Load, bool, false)
	GENERAL_ACCESSORS(RDTSCDriver, Unload, bool, false)
	GENERAL_ACCESSORS(RDTSCDriver, CustomName, std::string, "RDTSCEMU")
	// TODO: remove UseCustomName since its not needed anymore
	GENERAL_ACCESSORS(RDTSCDriver, UseCustomName, bool, false)
	GENERAL_ACCESSORS(RDTSCDriver, Delta, int, 1)

	GENERAL_ACCESSORS(StealthDriver, Load, bool, false)
	GENERAL_ACCESSORS(StealthDriver, Unload, bool, false)
	GENERAL_ACCESSORS(StealthDriver, CustomName, std::string, "STEALTHDRIVER")
	GENERAL_ACCESSORS(StealthDriver, NtQueryInformationProcess, bool, false)
	GENERAL_ACCESSORS(StealthDriver, NtSetInformationThread, bool, false)
	// TODO: remove UseCustomName since its not needed anymore
	GENERAL_ACCESSORS(StealthDriver, UseCustomName, bool, false)

	VALUE_ACCESSORS(GetTickCountDelta, int, 1)
	VALUE_ACCESSORS(RemoteTCPPort, int, 4242)

	static HideDebuggerProfile readProfileFromFile(const boost::filesystem::path& fileName);
	static HideDebuggerProfile readProfileByName(const std::string& profileName);
	static void writeProfileToFile(const HideDebuggerProfile& profile, const boost::filesystem::path& fileName);
	static void writeProfileByName(const HideDebuggerProfile& profile, const std::string& profileName);

private:
	boost::property_tree::ptree pt_;
};

bool deleteProfileByName(const std::string& profileName);
void setCurrentProfileName(const std::string& profileName);
boost::filesystem::path getCurrentProfileFile();
std::string getCurrentProfileName();
std::vector<std::string> getProfileNames();
void saveCurrentProfileName();

}