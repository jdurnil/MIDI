// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#pragma once

#include <queue>
#include <mutex>

class CMidiEndpointProtocolWorker : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IMidiCallback>
{
public:
    CMidiEndpointProtocolWorker() = default;
    ~CMidiEndpointProtocolWorker() {}

    STDMETHOD(Initialize)(
        _In_ GUID SessionId,
        _In_ GUID AbstractionGuid,
        _In_ LPCWSTR DeviceInterfaceId,
        _In_ std::shared_ptr<CMidiClientManager>& ClientManager,
        _In_ std::shared_ptr<CMidiDeviceManager>& DeviceManager,
        _In_ std::shared_ptr<CMidiSessionTracker>& SessionTracker
        );

    STDMETHOD(ListenForMetadata)();

    STDMETHOD(Callback)(_In_ PVOID Data, _In_ UINT Size, _In_ LONGLONG Position, _In_ LONGLONG Context);

    STDMETHOD(Cleanup)();

private:
    std::wstring m_deviceInterfaceId;
    std::wstring m_deviceInstanceId;
    GUID m_sessionId;
    GUID m_abstractionGuid;

    LONGLONG m_context{ 0 };

    wil::unique_event_nothrow m_allNegotiationMessagesReceived;
    wil::unique_event_nothrow m_queueWorkerThreadWakeup;

    // true if we're closing down
    bool m_shutdown{ false };

    //wil::com_ptr_nothrow<IMidiDeviceManagerInterface> m_MidiDeviceManager;
    std::shared_ptr<CMidiClientManager> m_clientManager;
    std::shared_ptr<CMidiDeviceManager> m_deviceManager;
    std::shared_ptr<CMidiSessionTracker> m_sessionTracker;

    wil::com_ptr_nothrow<IMidiBiDi> m_midiBiDiDevice;


    // these are holding locations while names are built
    std::wstring m_endpointName{};
    std::wstring m_productInstanceId{};
    std::map<uint8_t /* function block number */, std::wstring> m_functionBlockNames;


    std::wstring ParseStreamTextMessage(_In_ internal::PackedUmp128& message);

    DEVPROPKEY FunctionBlockPropertyKeyFromNumber(_In_ uint8_t functionBlockNumber);
    DEVPROPKEY FunctionBlockNamePropertyKeyFromNumber(_In_ uint8_t functionBlockNumber);

    HRESULT UpdateEndpointNameProperty();
    HRESULT UpdateEndpointProductInstanceIdProperty();
    HRESULT UpdateFunctionBlockNameProperty(_In_ uint8_t functionBlockNumber, _In_ std::wstring value);

    HRESULT UpdateDeviceIdentityProperty(_In_ internal::PackedUmp128& identityMessage);
    HRESULT UpdateEndpointInfoProperties(_In_ internal::PackedUmp128& endpointInfoNotificationMessage);
    HRESULT UpdateStreamConfigurationProperties(_In_ internal::PackedUmp128& endpointStreamConfigurationNotificationMessage);
    HRESULT UpdateFunctionBlockProperty(_In_ internal::PackedUmp128& functionBlockInfoNotificationMessage);

    HRESULT HandleFunctionBlockNameMessage(_In_ internal::PackedUmp128& functionBlockNameMessage);
    HRESULT HandleEndpointNameMessage(_In_ internal::PackedUmp128& endpointNameMessage);
    HRESULT HandleProductInstanceIdMessage(_In_ internal::PackedUmp128& productInstanceIdMessage);

    HRESULT ProcessStreamMessage(_In_ internal::PackedUmp128 ump);
};


