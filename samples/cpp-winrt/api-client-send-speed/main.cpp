﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

// Windows MIDI Services sample code

#include "pch.h"
#include <iostream>
#include <iomanip>

using namespace winrt::Windows::Devices::Midi2;        // API

// standard WinRT enumeration support. This is how you find attached devices.
using namespace winrt::Windows::Devices::Enumeration;

// where you find types like IAsyncOperation, IInspectable, etc.
namespace foundation = winrt::Windows::Foundation;
namespace collections = winrt::Windows::Foundation::Collections;

void DisplaySingleResult(std::wstring label, uint64_t totalTime, uint64_t errorCount, uint32_t totalMessageCount)
{
    std::wcout 
        << std::setw(35) << std::left
        << label
        << L"Total: "
        << std::fixed << std::setprecision(2) << std::right
        << MidiClock::ConvertTimestampToMilliseconds(totalTime)
        << "ms, average: "
        << std::fixed << std::setprecision(2) << std::right
        << MidiClock::ConvertTimestampToMicroseconds((uint64_t)((double)totalTime / totalMessageCount))
        << " microseconds per message, "
        << errorCount 
        << " errors"
        << std::endl;

}


int main()
{
    winrt::init_apartment();

    auto endpointId = MidiEndpointDeviceInformation::DiagnosticsLoopbackAEndpointId();
    uint32_t messagesPerIteration = 100;
    uint32_t iterations = 1000;

    uint64_t TotalSendTicksIndividualMessagePackets{ 0 };
    uint64_t TotalSendTicksIndividualMessageWords{ 0 };
    uint64_t TotalSendTicksIndividualMessageStructs{ 0 };

    uint64_t TotalSendTicksMultipleMessagePackets{ 0 };
    uint64_t TotalSendTicksMultipleMessageWords{ 0 };
    uint64_t TotalSendTicksMultipleMessageStructs{ 0 };
    uint64_t TotalSendTicksMultipleMessageBuffer{ 0 };

    uint64_t TotalSendErrorsIndividualMessagePackets{ 0 };
    uint64_t TotalSendErrorsIndividualMessageWords{ 0 };
    uint64_t TotalSendErrorsIndividualMessageStructs{ 0 };

    uint64_t TotalSendErrorsMultipleMessagePackets{ 0 };
    uint64_t TotalSendErrorsMultipleMessageWords{ 0 };
    uint64_t TotalSendErrorsMultipleMessageStructs{ 0 };
    uint64_t TotalSendErrorsMultipleMessageBuffer{ 0 };



    // create the MIDI session, giving us access to Windows MIDI Services. An app may open 
    // more than one session. If so, the session name should be meaningful to the user, like
    // the name of a browser tab, or a project.

    auto session = MidiSession::CreateSession(L"Speed Test");

    auto sendEndpoint = session.CreateEndpointConnection(endpointId);

    // once you have wired up all your event handlers, added any filters/listeners, etc.
    // You can open the connection. Doing this will query the cache for the in-protocol 
    // endpoint information and function blocks. If not there, it will send out the requests
    // which will come back asynchronously with responses.
    sendEndpoint.Open();

    std::cout << "Connected to open send endpoint: " << winrt::to_string(endpointId) << std::endl;


    auto ump64 = MidiMessageBuilder::BuildMidi2ChannelVoiceMessage(
        MidiClock::TimestampConstantSendImmediately(),
        5,      // group 5
        Midi2ChannelVoiceMessageStatus::NoteOn,    
        3,      // channel 3
        120,    // note 120 - hex 0x78
        100);   // velocity 100 hex 0x64

    auto ump32 = MidiMessageBuilder::BuildMidi1ChannelVoiceMessage(
        MidiClock::TimestampConstantSendImmediately(),
        5,      // group 5
        Midi1ChannelVoiceMessageStatus::NoteOn,    
        2,      // channel 3
        110,    // note 110
        200);   // velocity 200


    // Build our messages to send out. We do this all at once, and it is not part of the benchmarking
    // this way, we're *only* testing the send speed

    auto messageList = winrt::single_threaded_vector<IMidiUniversalPacket>() ;
    auto wordList = winrt::single_threaded_vector<uint32_t>();
    auto structList = winrt::single_threaded_vector<MidiMessageStruct>();
    auto wordListList = std::vector<collections::IVectorView<uint32_t>>();




    // if we change the types of messages we send, we need to change this as well
    foundation::MemoryBuffer buffer(messagesPerIteration * sizeof(uint32_t) * 3);

  
    uint32_t memoryBufferOffset = 0;
    uint32_t bytesWritten = 0;

    for (uint32_t i = 0; i < messagesPerIteration; i += 2)
    {
        // message list
        messageList.Append(ump64);
        messageList.Append(ump32);

        // word list
        ump64.AppendAllMessageWordsToVector(wordList);
        ump32.AppendAllMessageWordsToVector(wordList);

        // buffer
        ump64.AddAllMessageBytesToBuffer(buffer, memoryBufferOffset);
        ump32.AddAllMessageBytesToBuffer(buffer, memoryBufferOffset);

        // structs

        MidiMessageStruct str64;
        str64.Word0 = ump64.Word0();
        str64.Word1 = ump64.Word1();
        structList.Append(str64);

        MidiMessageStruct str32;
        str32.Word0 = ump32.Word0();
        structList.Append(str32);

        // for sending words one message at a time
        wordListList.push_back(ump64.GetAllWords());
        wordListList.push_back(ump32.GetAllWords());




        bytesWritten += sizeof(ump64) + sizeof(ump32);

        memoryBufferOffset += bytesWritten;
    }


    // Actually send the messages

    for (uint32_t i = 0; i < iterations; i++)
    {
        uint64_t startTick{};
        uint64_t stopTick{};

        MidiSendMessageResult result;

        // individual message tests ---------------------------------------------------------------------

        // send individual message words
        startTick = MidiClock::Now();

        for (auto const& message : wordListList)
        {
            MidiSendMessageResult result;

            if (message.Size() == 4)
                result = sendEndpoint.SendMessageWords(MidiClock::TimestampConstantSendImmediately(), message.GetAt(0), message.GetAt(1), message.GetAt(2), message.GetAt(3));
            else if (message.Size() == 3)
                result = sendEndpoint.SendMessageWords(MidiClock::TimestampConstantSendImmediately(), message.GetAt(0), message.GetAt(1), message.GetAt(2));
            else if (message.Size() == 2)
                result = sendEndpoint.SendMessageWords(MidiClock::TimestampConstantSendImmediately(), message.GetAt(0), message.GetAt(1));
            else if (message.Size() == 1)
                result = sendEndpoint.SendMessageWords(MidiClock::TimestampConstantSendImmediately(), message.GetAt(0));

            if (MidiEndpointConnection::SendMessageFailed(result))
            {
                TotalSendErrorsIndividualMessageWords++;
            }

        }
        stopTick = MidiClock::Now();
        TotalSendTicksIndividualMessageWords += (stopTick - startTick);



        // send individual structs

        startTick = MidiClock::Now();
        for (auto const& message : structList)
        {
            result = sendEndpoint.SendMessageStruct(MidiClock::TimestampConstantSendImmediately(), message, (uint8_t)MidiMessageUtility::GetPacketTypeFromMessageFirstWord(message.Word0));
            if (MidiEndpointConnection::SendMessageFailed(result))
            {
                TotalSendErrorsIndividualMessageStructs++;
            }
        }
        stopTick = MidiClock::Now();
        TotalSendTicksIndividualMessageStructs += (stopTick - startTick);



        // send individual message packets

        startTick = MidiClock::Now();
        for (auto const& message : messageList)
        {
            result = sendEndpoint.SendMessagePacket(message);
            if (MidiEndpointConnection::SendMessageFailed(result))
            {
                TotalSendErrorsIndividualMessagePackets++;
            }
        }
        stopTick = MidiClock::Now();
        TotalSendTicksIndividualMessagePackets += (stopTick - startTick);


        // send individual buffers





        // multi-message tests --------------------------------------------------------------------------

        // send vector of words

        startTick = MidiClock::Now();
        result = sendEndpoint.SendMultipleMessagesWordList(MidiClock::TimestampConstantSendImmediately(), wordList);
        if (MidiEndpointConnection::SendMessageFailed(result))
        {
            TotalSendErrorsMultipleMessageWords++;
        }
        stopTick = MidiClock::Now();
        TotalSendTicksMultipleMessageWords += (stopTick - startTick);


        // send vector of message packets

        startTick = MidiClock::Now();
        result = sendEndpoint.SendMultipleMessagesPacketList(messageList);
        if (MidiEndpointConnection::SendMessageFailed(result))
        {
            TotalSendErrorsMultipleMessagePackets++;
        }
        stopTick = MidiClock::Now();
        TotalSendTicksMultipleMessagePackets += (stopTick - startTick);


        // send vector of message structs

        startTick = MidiClock::Now();
        result = sendEndpoint.SendMultipleMessagesStructList(MidiClock::TimestampConstantSendImmediately(), structList);
        if (MidiEndpointConnection::SendMessageFailed(result))
        {
            TotalSendErrorsMultipleMessageStructs++;
        }
        stopTick = MidiClock::Now();
        TotalSendTicksMultipleMessageStructs += (stopTick - startTick);


        // send multiple through buffer

        startTick = MidiClock::Now();
        result = sendEndpoint.SendMultipleMessagesBuffer(MidiClock::TimestampConstantSendImmediately(), buffer, 0, bytesWritten);
        if (MidiEndpointConnection::SendMessageFailed(result))
        {
            TotalSendErrorsMultipleMessageBuffer++;
        }
        stopTick = MidiClock::Now();
        TotalSendTicksMultipleMessageBuffer += (stopTick - startTick);
    }


    // let's see how we did

    uint32_t totalMessageCount = iterations * messagesPerIteration;

    std::wcout << std::endl;
    std::wcout << L"Iteration Count:        " << iterations << std::endl;
    std::wcout << L"Messages per Iteration: " << messagesPerIteration << std::endl;
    std::wcout << L"Total Message Count:    " << messagesPerIteration * iterations << std::endl;

    std::wcout << std::endl << L"Multiple messages per call" << std::endl;
    std::wcout << L"----------------------------------------------------------------------------------------" << std::endl;
    DisplaySingleResult(L"SendMultipleMessagesPacketList", TotalSendTicksMultipleMessagePackets, TotalSendErrorsMultipleMessagePackets, totalMessageCount);
    DisplaySingleResult(L"SendMultipleMessagesStructList", TotalSendTicksMultipleMessageStructs, TotalSendErrorsMultipleMessageStructs, totalMessageCount);
    DisplaySingleResult(L"SendMultipleMessagesWordList", TotalSendTicksMultipleMessageWords, TotalSendErrorsMultipleMessageWords, totalMessageCount);
    DisplaySingleResult(L"SendMultipleMessagesBuffer", TotalSendTicksMultipleMessageBuffer, TotalSendErrorsMultipleMessageBuffer, totalMessageCount);

    std::wcout << std::endl << L"Single message per call" << std::endl;
    std::wcout << L"----------------------------------------------------------------------------------------" << std::endl;
    DisplaySingleResult(L"SendMessagePacket", TotalSendTicksIndividualMessagePackets, TotalSendErrorsIndividualMessagePackets, totalMessageCount);
    DisplaySingleResult(L"SendMessageStruct", TotalSendTicksIndividualMessageStructs, TotalSendErrorsIndividualMessageStructs, totalMessageCount);
    DisplaySingleResult(L"SendMessageWords", TotalSendTicksIndividualMessageWords, TotalSendErrorsIndividualMessageWords, totalMessageCount);

    // shut down

    std::cout << std::endl << "Disconnecting UMP Endpoint Connection..." << std::endl;

    session.DisconnectEndpointConnection(sendEndpoint.ConnectionId());
    // close the session, detaching all Windows MIDI Services resources and closing all connections
    // You can also disconnect individual Endpoint Connections when you are done with them, as we did above
    session.Close();

}
