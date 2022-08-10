﻿using Microsoft.Windows.Midi.PluginModel.Device;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Windows.Midi.PluginModel.Plugin
{
    public interface IMidiDevicePlugin
    {
        MidiPluginMetadata PluginMetadata { get; }

        IMidiDeviceFactory GetDeviceFactory();

        IMidiDeviceEnumerator GetDeviceEnumerator();

    }
}
