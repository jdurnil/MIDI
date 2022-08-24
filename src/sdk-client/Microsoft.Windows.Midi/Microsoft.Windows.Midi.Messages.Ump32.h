#pragma once
#include "Microsoft.Windows.Midi.Messages.Ump32.g.h"
#include "Microsoft.Windows.Midi.Messages.Ump.h"

// WARNING: This file is automatically generated by a tool. Do not directly
// add this file to your project, as any changes you make will be lost.
// This file is a stub you can use as a starting point for your implementation.
//
// To add a copy of this file to your project:
//   1. Copy this file from its original location to the location where you store 
//      your other source files (e.g. the project root). 
//   2. Add the copied file to your project. In Visual Studio, you can use 
//      Project -> Add Existing Item.
//   3. Delete this comment and the 'static_assert' (below) from the copied file.
//      Do not modify the original file.
//
// To update an existing file in your project:
//   1. Copy the relevant changes from this file and merge them into the copy 
//      you made previously.
//    
// This assertion helps prevent accidental modification of generated files.
//static_assert(false, "This file is generated by a tool and will be overwritten. Open this error and view the comment for assistance.");

namespace winrt::Microsoft::Windows::Midi::Messages::implementation
{
    struct Ump32 : Ump32T<Ump32, Microsoft::Windows::Midi::Messages::implementation::Ump>
    {
        Ump32() = default;

        Ump32(uint32_t word0);
        static winrt::Microsoft::Windows::Midi::Messages::Ump32 FromWord(uint32_t word0);
    };
}
namespace winrt::Microsoft::Windows::Midi::Messages::factory_implementation
{
    struct Ump32 : Ump32T<Ump32, implementation::Ump32>
    {
    };
}