// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#include "pch.h"
#include "MidiControlChangeMessage.h"
#include "MidiControlChangeMessage.g.cpp"


namespace winrt::MIDI_ROOT_NAMESPACE_CPP::implementation
{
    _Use_decl_annotations_
    MidiControlChangeMessage::MidiControlChangeMessage(uint8_t /*channel*/, uint8_t /*controller*/, uint8_t /*controlValue*/)
    {
        throw hresult_not_implemented();
    }

    uint8_t MidiControlChangeMessage::Channel()
    {
        throw hresult_not_implemented();
    }
    uint8_t MidiControlChangeMessage::Controller()
    {
        throw hresult_not_implemented();
    }
    uint8_t MidiControlChangeMessage::ControlValue()
    {
        throw hresult_not_implemented();
    }
    foundation::TimeSpan MidiControlChangeMessage::Timestamp()
    {
        throw hresult_not_implemented();
    }
    midi1::MidiMessageType MidiControlChangeMessage::Type()
    {
        throw hresult_not_implemented();
    }
    streams::IBuffer MidiControlChangeMessage::RawData()
    {
        throw hresult_not_implemented();
    }

}