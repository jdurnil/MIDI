// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#pragma once
#include "MidiOutputEndpointConnection.g.h"
#include "MidiEndpointConnection.h"
#include "midi_service_interface.h"

namespace winrt::Windows::Devices::Midi2::implementation
{
    struct MidiOutputEndpointConnection : MidiOutputEndpointConnectionT<MidiOutputEndpointConnection, Windows::Devices::Midi2::implementation::MidiEndpointConnection>
    {
        MidiOutputEndpointConnection() = default;
        static hstring GetDeviceSelectorForOutput() { return L""; /* TODO */ }

        bool SendUmp(_In_ winrt::Windows::Devices::Midi2::IMidiUmp const& ump);
        bool SendUmpWordArray(_In_ internal::MidiTimestamp timestamp, _In_ array_view<uint32_t const> words, _In_ uint32_t wordCount);

        bool SendUmpBuffer(_In_ internal::MidiTimestamp timestamp, _In_ winrt::Windows::Foundation::IMemoryBuffer const& buffer, _In_ uint32_t byteOffset, _In_ uint32_t byteLength);

        bool SendUmpWords(_In_ internal::MidiTimestamp timestamp, _In_ uint32_t word0);
        bool SendUmpWords(_In_ internal::MidiTimestamp timestamp, _In_ uint32_t word0, _In_ uint32_t word1);
        bool SendUmpWords(_In_ internal::MidiTimestamp timestamp, _In_ uint32_t word0, _In_ uint32_t word1, _In_ uint32_t word2);
        bool SendUmpWords(_In_ internal::MidiTimestamp timestamp, _In_ uint32_t word0, _In_ uint32_t word1, _In_ uint32_t word2, _In_ uint32_t word3);

        bool InternalStart();

    private:

    };
}
namespace winrt::Windows::Devices::Midi2::factory_implementation
{
    struct MidiOutputEndpointConnection : MidiOutputEndpointConnectionT<MidiOutputEndpointConnection, implementation::MidiOutputEndpointConnection>
    {
    };
}
