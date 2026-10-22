export module Contract.Audio.Service:VoiceHandle;

import Core.Handle;

export struct VoiceTag {};
export using VoiceHandle = Core::GenerationalHandle<VoiceTag>;