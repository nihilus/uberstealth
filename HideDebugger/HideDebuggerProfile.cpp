#pragma warning(disable : 4512)
#include <boost/property_tree/xml_parser.hpp>
#pragma warning(default : 4512)
#include "HideDebuggerProfile.h"
#include <ShlObj.h>

namespace
{
	boost::filesystem::path profilesPath = uberstealth::ProfileHelper::getConfigPath();

	// Get the absolute path for a given profile filename.
	boost::filesystem::path getProfileFile(const std::string& profileName)
	{
		return (profilesPath / boost::filesystem::path(profileName)).replace_extension(".xml");
	}

	const char* MainConfigFile = "uberstealth.config";
}

// Map given name to the corresponding file in the standard profiles path and read the profile.
uberstealth::HideDebuggerProfile uberstealth::HideDebuggerProfile::readProfileByName(const std::string& profileName)
{
	boost::property_tree::ptree pt;
	boost::property_tree::xml_parser::read_xml(getProfileFile(profileName).string(), pt, boost::property_tree::xml_parser::trim_whitespace);
	return HideDebuggerProfile(pt);
}

// Read profile from the given filename.
uberstealth::HideDebuggerProfile uberstealth::HideDebuggerProfile::readProfile(const std::string& fileName)
{
	boost::property_tree::ptree pt;
	boost::property_tree::xml_parser::read_xml(fileName, pt, boost::property_tree::xml_parser::trim_whitespace);
	return HideDebuggerProfile(pt);
}

// Write profile to given filename.
void uberstealth::HideDebuggerProfile::writeProfile(const HideDebuggerProfile& profile, const std::string& fileName)
{
	boost::property_tree::xml_parser::write_xml(fileName, 
												profile.pt_,
												std::locale(),
												boost::property_tree::xml_writer_settings<char>(' ', 4));
}

// Map profile name to the corresponding file in the standard profiles path and write the profile.
void uberstealth::HideDebuggerProfile::writeProfileByName(const HideDebuggerProfile& profile, const std::string& profileName)
{
	boost::property_tree::xml_parser::write_xml(getProfileFile(profileName).string(), 
												profile.pt_,
												std::locale(),
												boost::property_tree::xml_writer_settings<char>(' ', 4));
}

// Enumerate all profile files in the standard profiles path.
std::vector<std::string> uberstealth::HideDebuggerProfile::getProfiles()
{
	std::vector<std::string> profiles;
	boost::filesystem::directory_iterator end;
	for(boost::filesystem::directory_iterator iter(profilesPath); iter!=end ; ++iter)
	{
		const boost::filesystem::path& p = iter->path();
		if (boost::filesystem::is_regular_file(p) && p.extension() == ".xml")
		{
			profiles.push_back(p.filename().string());
		}
	}

	return profiles;
}

bool uberstealth::HideDebuggerProfile::deleteProfileByName(const std::string& profileName)
{
	return boost::filesystem::remove(getProfileFile(profileName));
}

// ProfileHelper

uberstealth::ProfileHelper::ProfileHelper()
{
	profilesPath_ = getConfigPath();
	mainConfigFile_ = profilesPath_ / MainConfigFile;
	
	// Create an empty default profile file if it doesn't exist yet.
	boost::filesystem::path defaultFile = profilesPath_ / "default.xml";
	if (!boost::filesystem::exists(defaultFile))
	{
		uberstealth::HideDebuggerProfile::writeProfile(HideDebuggerProfile(), defaultFile.string());
	}
	lastProfile_ = readLastProfile(mainConfigFile_);
	if (lastProfile_.empty())
	{
		lastProfile_ = "default.xml";
	}
}

// Determine the directory where all profile files are stored.
// Note: this function must not throw since it might be invoked from static initializers.
boost::filesystem::path uberstealth::ProfileHelper::getConfigPath()
{
	using namespace boost::filesystem;
	try
	{
		wchar_t appDataPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, appDataPath)))
		{
			path p(appDataPath);
			p /= "uberstealth";
			if (!exists(p)) create_directory(p);
			return p;
		}
	}
	catch (const std::exception&)
	{
	}
	return path();
}

std::string uberstealth::ProfileHelper::readLastProfile(const boost::filesystem::path& mainConfigFile) const
{
	std::ifstream ifs(mainConfigFile.string().c_str());
	std::string lastProfile;
	ifs >> lastProfile;
	return lastProfile;
}

void uberstealth::ProfileHelper::writeLastProfile(const std::string& lastProfile, const boost::filesystem::path& mainConfigFile) const
{
	std::ofstream ofs(mainConfigFile.string().c_str());
	ofs << lastProfile;
}

void uberstealth::ProfileHelper::writeLastProfile()
{
	writeLastProfile(lastProfile_, mainConfigFile_);
}