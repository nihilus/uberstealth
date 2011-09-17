#pragma once

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <RDTSCEmu/driver/RDTSCEmu.h>
#include <vector>

namespace uberstealth {

enum InlinePatching { AutoSelect, ForceAbsolute };

// Represents the settings of a single configuration profile.
class HideDebuggerProfile
{
#define GENERAL_ACCESSORS(config_name, config_path, data_type, default_value) data_type get##config_name##config_path() const { return pt_.get(#config_name"."#config_path, default_value); } \
																			  void set##config_name##config_path(const data_type& value) { pt_.put(#config_name"."#config_path, value); }

// Same as GENERAL_ACCESSORS but also expects a translator so the corresponding type can be represented as std::string.
#define TRANSLATOR_ACCESSORS(config_name, config_path, data_type, default_value, translator) data_type get##config_name##config_path() const { return pt_.get(#config_name"."#config_path, default_value, translator##()); } \
																							 void set##config_name##config_path(data_type value) { pt_.put(#config_name"."#config_path, value, translator##()); }

#define ENABLED_ACCESSORS(config_name)									  GENERAL_ACCESSORS(config_name, Enabled, bool, false)
#define VALUE_ACCESSORS(config_name, data_type, default_value)			  GENERAL_ACCESSORS(config_name, Value, data_type, default_value)

private:
	// Custom property_tree translator for the InlinePatching enum.
	struct InlinePatchingTranslator
	{
		InlinePatching get_value(const std::string& value) const
		{
			if (value == "AutoSelect") return AutoSelect;
			else if (value == "ForceAbsolute")return ForceAbsolute;
			else throw std::runtime_error("Unable to convert given string to InlinePatching enum (" + value + ")");
		}

		std::string put_value(InlinePatching value) const
		{
			switch (value)
			{
			case AutoSelect: return "AutoSelect";
			case ForceAbsolute: return "ForceAbsolute";
			default: throw std::runtime_error("Unable to convert unknown InlinePatching enum to string");
			}
		}
	};

	// Custom property_tree translator for the RDTSCMode enum.
	struct RDTSCModeTranslator
	{
		RDTSCMode get_value(const std::string& value) const
		{
			if (value == "Constant") return constant;
			else if (value == "Increasing")return increasing;
			else throw std::runtime_error("Unable to convert given string to RDTSCMode enum (" + value + ")");
		}

		std::string put_value(RDTSCMode value) const
		{
			switch (value)
			{
			case constant: return "Constant";
			case increasing: return "Increasing";
			default: throw std::runtime_error("Unable to convert unknown RDTSCMode enum to string");
			}
		}
	};

public:
	HideDebuggerProfile(const boost::property_tree::ptree& pt) : pt_(pt) {}
	HideDebuggerProfile() {};

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

	static HideDebuggerProfile readProfile(const std::string& fileName);
	static HideDebuggerProfile readProfileByName(const std::string& profileName);
	static void writeProfile(const HideDebuggerProfile& profile, const std::string& fileName);
	static void writeProfileByName(const HideDebuggerProfile& profile, const std::string& profileName);
	static bool deleteProfileByName(const std::string& profileName);
	static std::vector<std::string> getProfiles();

private:
	boost::property_tree::ptree pt_;
};

// Provides some helper functionality for retrieving the profiles path and storing the profile which was used in the last session.
class ProfileHelper
{
public:
	ProfileHelper();
	static boost::filesystem::path getConfigPath();
	// Returns the filename of the profile used in the last session.
	std::string getLastProfileFilename() const { return lastProfile_; }
	void setLastProfileFilename(const std::string& profileFilename) { lastProfile_ = profileFilename; }
	// Returns the full path of the profile used in the last session.
	std::string getLastProfilePath() const { return (profilesPath_ / lastProfile_).string(); }
	// Write the last used profile to the main config file.
	void writeLastProfile();

private:
	std::string readLastProfile(const boost::filesystem::path& mainConfigFile) const;
	void writeLastProfile(const std::string& lastProfile, const boost::filesystem::path& mainConfigFile) const;

	boost::filesystem::path profilesPath_;
	boost::filesystem::path mainConfigFile_;
	std::string lastProfile_;
};

}