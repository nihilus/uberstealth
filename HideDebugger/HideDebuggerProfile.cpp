#include "HideDebuggerProfile.h"
#include <ShlObj.h>
#pragma warning(disable : 4512)
#include <boost/property_tree/xml_parser.hpp>
#pragma warning(default : 4512)

namespace {

const char* MainConfigFile = "uberstealth.config";
const char* DefaultProfileFile = "default.xml";

std::string currentProfileName_;

const boost::filesystem::path& getProfilesPath() {
	static boost::filesystem::path profilesPath;
	if (profilesPath.empty()) {
		wchar_t appDataPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, appDataPath))) {
			profilesPath = boost::filesystem::path(appDataPath) / "uberstealth";
		} else {
			throw std::runtime_error("Unable to determine application data path.");
		}
		if (!boost::filesystem::exists(profilesPath)) {
			create_directory(profilesPath);
		}
		//// Create an empty default profile file if it doesn't exist yet.
		//boost::filesystem::path defaultFile = profilesPath_ / DefaultProfileFile;
		//if (!boost::filesystem::exists(defaultFile)) {
		//	uberstealth::HideDebuggerProfile::writeProfile(HideDebuggerProfile(), UnicodeToString(defaultFile.wstring()));
		//}
	}
	return profilesPath;
}

const boost::filesystem::path& getMainConfigFile() {
	static boost::filesystem::path mainfConfigFile;
	if (mainfConfigFile.empty()) {
		mainfConfigFile = getProfilesPath() / MainConfigFile;
	}
	return mainfConfigFile;
}

std::string determineCurrentProfile() {
	std::ifstream ifs(getMainConfigFile().string().c_str());
	std::string lastProfile;
	ifs >> lastProfile;
	return lastProfile;
}

// Get the absolute path for a given profile filename.
boost::filesystem::path getAbsolutePath(const std::string& profileName) {
	return (getProfilesPath() / boost::filesystem::path(profileName)).replace_extension(".xml");
}

}

void uberstealth::setCurrentProfileName(const std::string& profileName) {
	currentProfileName_ = profileName;
}

boost::filesystem::path uberstealth::getCurrentProfileFile() {
	return getProfilesPath() / getCurrentProfileName();
}

std::string uberstealth::getCurrentProfileName() {
	if (currentProfileName_.empty()) {
		currentProfileName_ = determineCurrentProfile();
		if (currentProfileName_.empty()) {
			currentProfileName_ = DefaultProfileFile;
		}
	}
	return currentProfileName_;
}

std::vector<std::string> uberstealth::getProfileNames() {
	std::vector<std::string> profiles;
	boost::filesystem::directory_iterator end;
	for (boost::filesystem::directory_iterator iter(getProfilesPath()); iter!=end ; ++iter) {
		const boost::filesystem::path& p = iter->path();
		if (boost::filesystem::is_regular_file(p) && p.extension() == ".xml") {
			profiles.push_back(p.filename().string());
		}
	}
	return profiles;
}

bool uberstealth::deleteProfileByName(const std::string& profileName) {
	return boost::filesystem::remove(getAbsolutePath(profileName));
}

void uberstealth::saveCurrentProfileName() {
	std::ofstream ofs(getMainConfigFile().c_str());
	ofs << getCurrentProfileName();
}

// HideDebuggerProfile static functions.

uberstealth::HideDebuggerProfile uberstealth::HideDebuggerProfile::readProfileFromFile(const boost::filesystem::path& fileName) {
	boost::property_tree::ptree pt;
	boost::property_tree::xml_parser::read_xml(fileName.string(), pt, boost::property_tree::xml_parser::trim_whitespace);
	return HideDebuggerProfile(pt);
}

uberstealth::HideDebuggerProfile uberstealth::HideDebuggerProfile::readProfileByName(const std::string& profileName) {
	return readProfileFromFile(getAbsolutePath(profileName));
}

void uberstealth::HideDebuggerProfile::writeProfileToFile(const HideDebuggerProfile& profile, const boost::filesystem::path& fileName) {
	boost::property_tree::xml_parser::write_xml(fileName.string(), 
		profile.pt_,
		std::locale(),
		boost::property_tree::xml_writer_settings<char>(' ', 4));
}

void uberstealth::HideDebuggerProfile::writeProfileByName(const HideDebuggerProfile& profile, const std::string& profileName) {
	boost::property_tree::xml_parser::write_xml(getAbsolutePath(profileName).string(), 
		profile.pt_,
		std::locale(),
		boost::property_tree::xml_writer_settings<char>(' ', 4));
}
