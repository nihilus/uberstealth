// All classes which want to be informed about profile events need to implement this interface.

#pragma once

namespace uberstealth {

class ProfileEventConsumer
{
public:
	// The consumer should update its internal state based on the given profile.
	virtual void loadProfile(const uberstealth::HideDebuggerProfile& profile) =0;

	// The consumer should update the given profile regarding to its internal state.
	// If its internal state cannot be mapped to the supplied profile it should return false and the profile is not saved (e.g. invalid characters entered by the user).
	virtual void flushProfile(uberstealth::HideDebuggerProfile& profile) =0;

	// The consumer should update its controls according to the given enable state.
	virtual void changeGlobalEnableState(bool globalEnable) =0;

	// The consumer needs to tell whether its internal state has unmodified changes.
	virtual bool isProfileDirty() =0; 
};

}