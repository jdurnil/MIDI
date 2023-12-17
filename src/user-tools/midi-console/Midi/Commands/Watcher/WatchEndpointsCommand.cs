﻿using Spectre.Console.Cli;
using Spectre.Console;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel;

using Microsoft.Devices.Midi2.ConsoleApp.Resources;
using Windows.Devices.Midi2;

namespace Microsoft.Devices.Midi2.ConsoleApp
{
    internal class WatchEndpointsCommand : Command<WatchEndpointsCommand.Settings>
    {
        private MidiEndpointDeviceWatcher _watcher;
        internal class Settings : CommandSettings
        {
            [LocalizedDescription("ParameterEnumEndpointsIncludeLoopbackEndpoints")]
            [CommandOption("-l|--include-loopback")]
            [DefaultValue(false)]
            public bool IncludeDiagnosticLoopback { get; set; }

        }

        public override int Execute(CommandContext context, Settings settings)
        {
            MidiEndpointDeviceInformationFilter filter = 
                MidiEndpointDeviceInformationFilter.IncludeClientUmpNative | 
                MidiEndpointDeviceInformationFilter.IncludeClientByteStreamNative | 
                MidiEndpointDeviceInformationFilter.IncludeVirtualDeviceResponder;

            if (settings.IncludeDiagnosticLoopback)
            {
                filter |= MidiEndpointDeviceInformationFilter.IncludeDiagnosticLoopback;
            }

            _watcher = MidiEndpointDeviceWatcher.CreateWatcher(filter);


            _watcher.Added += OnWatcherDeviceAdded;
            _watcher.Removed += OnWatcherDeviceRemoved;
            _watcher.Updated += OnWatcherDeviceUpdated;

            _watcher.EnumerationCompleted += OnWatcherEnumerationCompleted;
            _watcher.Stopped += OnWatcherStopped;

            AnsiConsole.MarkupLine(Strings.WatcherPressEscapeToStopWatchingMessage);
            AnsiConsole.WriteLine();

            _watcher.Start();


            bool continueWaiting = true;

            while (continueWaiting)
            {
                if (Console.KeyAvailable)
                {
                    var keyInfo = Console.ReadKey(false);

                    if (keyInfo.Key == ConsoleKey.Escape)
                    {
                        continueWaiting = false;

                        // leading space is because the "E" in "Escape" is often lost in the output for some reason.
                        AnsiConsole.MarkupLine(" " + Strings.WatcherEscapePressedMessage);
                        break;
                    }
                }

                Thread.Sleep(0);
            }

            _watcher.Stop();


            _watcher.Added -= OnWatcherDeviceAdded;
            _watcher.Removed -= OnWatcherDeviceRemoved;
            _watcher.Updated -= OnWatcherDeviceUpdated;

            _watcher.EnumerationCompleted -= OnWatcherEnumerationCompleted;
            _watcher.Stopped -= OnWatcherStopped;


            return (int)MidiConsoleReturnCode.Success;
        }

        private void OnWatcherStopped(MidiEndpointDeviceWatcher sender, object args)
        {
            AnsiConsole.MarkupLine("Watcher Stopped");
        }

        private void OnWatcherEnumerationCompleted(MidiEndpointDeviceWatcher sender, object args)
        {
            AnsiConsole.MarkupLine("Enumeration Completed");
        }


        private void OnWatcherDeviceRemoved(MidiEndpointDeviceWatcher sender, Windows.Devices.Enumeration.DeviceInformationUpdate args)
        {
            AnsiConsole.MarkupLine("Removed: " + AnsiMarkupFormatter.FormatFullEndpointInterfaceId(args.Id));
        }

        private void OnWatcherDeviceUpdated(MidiEndpointDeviceWatcher sender, Windows.Devices.Enumeration.DeviceInformationUpdate args)
        {
            AnsiConsole.MarkupLine("Updated: " + AnsiMarkupFormatter.FormatFullEndpointInterfaceId(args.Id));
        }

        private void OnWatcherDeviceAdded(MidiEndpointDeviceWatcher sender, MidiEndpointDeviceInformation args)
        {
            AnsiConsole.MarkupLine("Added:   " + AnsiMarkupFormatter.FormatEndpointName(args.Name));
            AnsiConsole.MarkupLine("         " + AnsiMarkupFormatter.FormatFullEndpointInterfaceId(args.Id));
            AnsiConsole.MarkupLine("         " + args.EndpointPurpose.ToString());
        }
    }
}