// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services Client SDK and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================


#include "pch.h"

#include "MidiSession.h"
#include "MidiSession.g.cpp"

namespace Shared = Microsoft::Devices::Midi2::Internal::Shared;

namespace winrt::Microsoft::Devices::Midi2::implementation
{
    winrt::Microsoft::Devices::Midi2::MidiSession MidiSession::CreateNewSession(hstring const& sessionName)
    {
        winrt::com_ptr<MidiSession> sessionImpl = winrt::make_self<MidiSession>();

        // TODO: Call APIs to create session, and handle any other construction/settings/etc.

        // TODO: Capture pid, app title, etc.

        sessionImpl->_connectedEndpoints = winrt::single_threaded_observable_vector<winrt::Microsoft::Devices::Midi2::MidiEndpoint>();

        // API calls succeeded so call it good and return it
        sessionImpl->_isOpen = true;

        // return the projected instance
        return (winrt::Microsoft::Devices::Midi2::MidiSession)(*sessionImpl);
    }

    bool MidiSession::IsOpen()
    {
        return _isOpen;
    }


    winrt::Windows::Foundation::Collections::IObservableVector<winrt::Microsoft::Devices::Midi2::MidiEndpoint> MidiSession::ConnectedEndpoints()
    {
        return _connectedEndpoints;
    }

    winrt::Microsoft::Devices::Midi2::MidiEndpoint MidiSession::ConnectToEndpoint(hstring const& midiEndpointId, bool routeIncomingMessagesToSession, winrt::Microsoft::Devices::Midi2::MidiEndpointConnectOptions const& options)
    {
        throw hresult_not_implemented();
    }


    void MidiSession::DisconnectFromEndpoint(hstring const& midiEndpointId)
    {
        throw hresult_not_implemented();
    }
    void MidiSession::Close()
    {
        // TODO: Call API to close the session and disconnect from the service

        _isOpen = false;
    }




    uint64_t MidiSession::GetMidiTimestamp()
    {
        return Shared::GetCurrentMidiTimestamp();
    }
    uint64_t MidiSession::GetMidiTimestampFrequency()
    {
        return Shared::GetMidiTimestampFrequency();
    }
}