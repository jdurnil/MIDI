﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Windows.Midi.Internal.ServiceProtocol.Midi
{
    public interface IMidiUmpMessageQueue : IDisposable
    {
        //int Count { get; }

        //uint GetRemainingCapacityInWords();
        //bool HasMessages();

        bool IsFull();

        bool IsEmpty();



       // void Initialize(int maxMidiWords, Guid id);

        // ump32, or just single words
        bool Enqueue(MidiWord word);

        bool Enqueue(ref Ump32 ump);

        // ump64, or any two words
        bool Enqueue(ref Ump64 ump);

        // ump96, or any three words
        bool Enqueue(ref Ump96 ump);

        // ump128 or any four words
        bool Enqueue(ref Ump128 ump);


        //bool EnqueueMany(Stream sourceStream);

        bool Enqueue(ref Span<MidiWord> words);


        bool Dequeue(out MidiWord word);

        bool Dequeue(out Ump32 ump);
        bool Dequeue(out Ump64 ump);
        bool Dequeue(out Ump96 ump);
        bool Dequeue(out Ump128 ump);

        //bool Dequeue(Stream destinationStream, out int wordCount);

        bool Dequeue(ref Span<MidiWord> words, out int wordCount);


        // method to help with removing a full message
        int PeekNextMessageWordCount();



        // may want to provide methods which operate on spans
        // and other types for block-copy


        // TODO: Provide a DequeueWholeMessage which will pull a message out using the 
        // message type to figure out how many words to read.

        // TODO: Provide an EnqueueWholeMessage which will write a full message


    }
}
