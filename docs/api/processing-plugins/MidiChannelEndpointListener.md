---
layout: api_page
title: MidiChannelEndpointListener
parent: Client Plugins
grand_parent: Windows.Devices.Midi2 API
has_children: false
---

# MidiChannelEndpointListener

This class acts as a filter. Incoming messages with the specified group and channel will be provided through the `MessageReceived` event. Other messages will be ignored.

In addition to the properties and methods in `IMidiEndpointMessageProcessingPlugin`, and the MessageReceived event from `IMidiMessageReceivedEventSource` the class provides the following:

## Properties

| Property | Description |
| ---- | ---- |
| `IncludeGroup` | The `MidiGroup` that this listener will listen to. |
| `IncludeChannels` | The channels that this listener will listen to on the group. |
| `PreventCallingFurtherListeners` | True if this plugin should prevent further listeners from processing a message that is in-scope for this processor. |
| `PreventFiringMainMessageReceivedEvent` | True if this plugin should prevent the endpoint's `MessageReceived` event from firing if the message was in-scope for this plugin. |

## Functions

| Property | Description |
| ---- | ---- |
| `MidiChannelEndpointListener()` | Construct a new instance of this type |

## Example

```cpp
// set up your message receive handler and create your connection
// before setting up the individual message listeners. The event
// handler has the same signature as the main message received
// event on the connection.

midi2::MidiChannelEndpointListener channelsListener;

// listening to channels generally only makes sense if you also
// specify the group you are listening to.
channelsListener.IncludeGroup(midi2::MidiGroup(5));

// add the channels you are listening to. Any messages which do 
// not have channels will not be raised through the event here.
channelsListener.IncludeChannels().Append(midi2::MidiChannel(3));
channelsListener.IncludeChannels().Append(midi2::MidiChannel(7));

// set this if you don't want the main message received event on the
// connection to fire for any messages this plugin handles.
channelsListener.PreventFiringMainMessageReceivedEvent(true);

auto channelMessagesReceivedEventToken = channelsListener.MessageReceived(MyMessageReceivedHandler);

myConnection.AddMessageProcessingPlugin(channelsListener);

// open after setting up the plugin so you don't miss any messages
myConnection.Open();

// ...
```

## IDL

[MidiChannelEndpointListener IDL](https://github.com/microsoft/MIDI/blob/main/src/api/Client/Midi2Client/MidiChannelEndpointListener.idl)
