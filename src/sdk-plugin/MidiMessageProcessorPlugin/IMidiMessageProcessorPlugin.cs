﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Windows.Midi.PluginModel.Plugin
{
    public interface IMidiMessageProcessorPlugin
    {

        MidiPluginMetadata PluginMetadata { get; }


    }
}
