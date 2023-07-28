// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#pragma once
#include "pch.h"

#include "MidiUmp64.g.h"


namespace winrt::Windows::Devices::Midi2::implementation
{
    struct MidiUmp64 : MidiUmp64T<MidiUmp64>
    {
        MidiUmp64() = default;
        MidiUmp64(internal::MidiTimestamp timestamp, uint32_t word0, uint32_t word1);

        // TODO: This doesn't do any bounds checking, and it should
        MidiUmp64(uint64_t timestamp, array_view<uint32_t const> words) : MidiUmp64(timestamp, (PVOID)words.data()) {};

        // internal
        MidiUmp64(internal::MidiTimestamp timestamp, PVOID data);


        uint32_t Word0() { return m_ump.word0; }
        void Word0(uint32_t value) { m_ump.word0 = value; }

        uint32_t Word1() { return m_ump.word1; }
        void Word1(uint32_t value) { m_ump.word1 = value; }

        internal::MidiTimestamp Timestamp() { return m_timestamp; }
        void Timestamp(internal::MidiTimestamp value) { m_timestamp = value; }

        winrt::Windows::Devices::Midi2::MidiUmpMessageType MessageType() { return (winrt::Windows::Devices::Midi2::MidiUmpMessageType)(internal::GetUmpMessageTypeFromFirstWord(m_ump.word0)); }
        void MessageType(winrt::Windows::Devices::Midi2::MidiUmpMessageType const& value) { internal::SetUmpMessageType(m_ump.word0, (uint8_t)value); }

        winrt::Windows::Devices::Midi2::MidiUmpPacketType MidiUmpPacketType() { return winrt::Windows::Devices::Midi2::MidiUmpPacketType::Ump64; }

    //    winrt::Windows::Foundation::IMemoryBuffer RawData();

        // internal for the sending code
        internal::PackedUmp64* GetInternalUmpDataPointer() { return &m_ump; }

    private:
        internal::MidiTimestamp m_timestamp{};

       // Windows::Foundation::MemoryBuffer _umpBackingStore = Windows::Foundation::MemoryBuffer(sizeof(internal::PackedUmp64));

        internal::PackedUmp64 m_ump{};

    };
}
namespace winrt::Windows::Devices::Midi2::factory_implementation
{
    struct MidiUmp64 : MidiUmp64T<MidiUmp64, implementation::MidiUmp64>
    {
    };
}