// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#pragma once
#include "MidiStartMessage.g.h"


namespace winrt::MIDI_ROOT_NAMESPACE_CPP::implementation
{
    struct MidiStartMessage : MidiStartMessageT<MidiStartMessage>
    {
        MidiStartMessage() = default;

        foundation::TimeSpan Timestamp();
        midi1::MidiMessageType Type();
        streams::IBuffer RawData();

    };
}
namespace winrt::MIDI_ROOT_NAMESPACE_CPP::factory_implementation
{
    struct MidiStartMessage : MidiStartMessageT<MidiStartMessage, implementation::MidiStartMessage>
    {
    };
}