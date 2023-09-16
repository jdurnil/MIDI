// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#pragma once
#include "MidiGlobalInformationCacheUpdatedEventArgs.g.h"

namespace winrt::Windows::Devices::Midi2::implementation
{
    struct MidiGlobalInformationCacheUpdatedEventArgs : MidiGlobalInformationCacheUpdatedEventArgsT<MidiGlobalInformationCacheUpdatedEventArgs>
    {
        MidiGlobalInformationCacheUpdatedEventArgs() = default;

        midi2::MidiCacheUpdateType UpdateType() { return m_updateType; }
        winrt::hstring Key() { return m_key; }

    private:
        midi2::MidiCacheUpdateType m_updateType;
        winrt::hstring m_key{};
    };
}
