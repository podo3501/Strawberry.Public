export module Client.Audio:VoiceHandle;

import Core.Handle;

export struct VoiceTag {};
export using VoiceHandle = Core::GenerationalHandle<VoiceTag>;