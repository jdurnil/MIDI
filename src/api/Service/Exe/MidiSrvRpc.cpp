// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License
// ============================================================================
// This is part of the Windows MIDI Services App API and should be used
// in your Windows application via an official binary distribution.
// Further information: https://github.com/microsoft/MIDI/
// ============================================================================

#include "stdafx.h"



_Use_decl_annotations_
void *midl_user_allocate(size_t size)
{
    return (void*)new (std::nothrow) BYTE[size];
}

_Use_decl_annotations_
void midl_user_free(void* p)
{
    delete[] (BYTE*)p;
}

// To be removed
// Simple test RPC to validate calling into the service and getting a service allocated
// structure returned to the client with valid contents. Simple POC of the midisrv RPC implementation.
HRESULT MidiSrvTestRpc(
    /* [in] */ handle_t /* BindingHandle */,
    /* [string][in] */ __RPC__in_string LPCWSTR Identifier,
    /* [in] */ __RPC__in PREQUESTED_SETTINGS RequestedSettings,
    /* [out] */ __RPC__deref_out_opt PALLOCATED_SETTINGS* AllocatedSettings)
{
    PALLOCATED_SETTINGS allocatedSettings = nullptr;
    GUID identifierGuid{0};

    // retrieve a shared pointer to the device manager, to confirm
    // that it can be retrieved.
    std::shared_ptr<CMidiDeviceManager> manager;

    auto cleanupOnExit = wil::scope_exit([&]() {
        if (allocatedSettings)
        {
            MIDL_user_free(allocatedSettings);
        }
    });

    RETURN_IF_FAILED(g_MidiService->GetDeviceManager(manager));

    // allocate our return structure
    allocatedSettings = (PALLOCATED_SETTINGS) MIDL_user_allocate(sizeof(ALLOCATED_SETTINGS));
    RETURN_IF_NULL_ALLOC(allocatedSettings);

    // fill in the return structure
    RETURN_IF_FAILED(CLSIDFromString(Identifier, &(allocatedSettings->Identifier)));
    allocatedSettings->Flag = RequestedSettings->Flag;
    allocatedSettings->Request = RequestedSettings->Request;
    RETURN_HR_IF(E_FAIL, RequestedSettings->Identifier != allocatedSettings->Identifier);

    // transfer the allocation
    *AllocatedSettings = allocatedSettings;
    allocatedSettings = nullptr;

    return S_OK;
}


HRESULT MidiSrvVerifyConnectivity(
    handle_t /*BindingHandle*/)
{
    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO)
    );

    std::shared_ptr<CMidiSessionTracker> sessionTracker;

    auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    // Client manager creates the client, fills in the MIDISRV_CLIENT information
    RETURN_IF_FAILED(g_MidiService->GetSessionTracker(sessionTracker));

    return sessionTracker->VerifyConnectivity();
}

HRESULT MidiSrvCreateClient(
    /* [in] */ handle_t BindingHandle,
    /* [string][in] */ __RPC__in_string LPCWSTR MidiDevice,
    /* [in] */ __RPC__in PMIDISRV_CLIENTCREATION_PARAMS CreationParams,
    /* [in] */ __RPC__in GUID SessionId,
    /* [out] */ __RPC__deref_out_opt PMIDISRV_CLIENT *Client
    )
{
    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO)
    );

    std::shared_ptr<CMidiClientManager> clientManager;
    PMIDISRV_CLIENT createdClient {nullptr};

    auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    auto cleanupOnExit = wil::scope_exit([&]() {
        if (createdClient)
        {
            MIDL_user_free(createdClient);
        }
    });

    // allocate a midl object to fill in and return to the client
    createdClient = (PMIDISRV_CLIENT) MIDL_user_allocate(sizeof(MIDISRV_CLIENT));
    RETURN_IF_NULL_ALLOC(createdClient);

    ZeroMemory(createdClient, sizeof(MIDISRV_CLIENT));

    // Client manager creates the client, fills in the MIDISRV_CLIENT information
    RETURN_IF_FAILED(g_MidiService->GetClientManager(clientManager));
    RETURN_IF_FAILED(clientManager->CreateMidiClient(BindingHandle, MidiDevice, SessionId, CreationParams, createdClient, false));

    // Success, transfer the MIDISRV_CLIENT data to the caller.
    *Client = createdClient;
    createdClient = nullptr;

    return S_OK;
}

HRESULT MidiSrvDestroyClient(
    /* [in] */ handle_t BindingHandle,
    /* [in] */ __RPC__in MidiClientHandle ClientHandle)
{
    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO)
    );

    std::shared_ptr<CMidiClientManager> clientManager;

    auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    // Client manager creates the client, fills in the MIDISRV_CLIENT information
    RETURN_IF_FAILED(g_MidiService->GetClientManager(clientManager));
    RETURN_IF_FAILED(clientManager->DestroyMidiClient(BindingHandle, ClientHandle));






    return S_OK;
}


HRESULT 
MidiSrvUpdateConfiguration(
    /* [in] */ handle_t BindingHandle,
    /*[in, string]*/ __RPC__in_string LPCWSTR ConfigurationJson,
    __RPC__in BOOL IsFromConfigurationFile,
    __RPC__out BSTR* Response)
{
    UNREFERENCED_PARAMETER(BindingHandle);

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Enter")
    );

    RETURN_HR_IF_NULL(E_INVALIDARG, ConfigurationJson);
    RETURN_HR_IF_NULL(E_INVALIDARG, Response);

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(ConfigurationJson, "JSON")
    );

    // Send it to the configuration manager and get it broken apart

    std::shared_ptr<CMidiConfigurationManager> configurationManager;
    std::shared_ptr<CMidiDeviceManager> deviceManager;

    auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    RETURN_IF_FAILED(g_MidiService->GetConfigurationManager(configurationManager));
    RETURN_IF_FAILED(g_MidiService->GetDeviceManager(deviceManager));

    auto configEntries = configurationManager->GetTransportAbstractionSettingsFromJsonString(ConfigurationJson);

    if (configEntries.size() == 0)
    {
        TraceLoggingWrite(
            MidiSrvTelemetryProvider::Provider(),
            __FUNCTION__,
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
            TraceLoggingWideString(L"No config entries in JSON string")
        );

        return E_FAIL;
    }

    CComBSTR combinedResponse;
    combinedResponse.Empty();

    for (auto i = configEntries.begin(); i != configEntries.end(); i++)
    {
        CComBSTR response;
        response.Empty();

        deviceManager->UpdateAbstractionConfiguration(i->first, i->second.c_str(), IsFromConfigurationFile, &response);

        // Probably need to do more formatting of this. Should use the json objects instead

        combinedResponse += response;

        ::SysFreeString(response);
    }

    RETURN_IF_FAILED(combinedResponse.CopyTo(Response));

    // TODO: Now check to see if it has settings for anything else, and send those along to be processed

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Exit success")
    );

    return S_OK;
}


HRESULT
MidiSrvRegisterSession(
    /* [in] */ handle_t BindingHandle,
    __RPC__in GUID SessionId,
    __RPC__in_string LPCWSTR SessionName,
    __RPC__in DWORD ProcessId,
    __RPC__in_string LPCWSTR ProcessName
)
{
    UNREFERENCED_PARAMETER(BindingHandle);

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Enter")
    );

    std::shared_ptr<CMidiSessionTracker> sessionTracker;

    auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    RETURN_IF_FAILED(g_MidiService->GetSessionTracker(sessionTracker));

    RETURN_IF_FAILED(sessionTracker->AddClientSession(SessionId, SessionName, ProcessId, ProcessName));

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Exit success")
    );

    return S_OK;
}

HRESULT
MidiSrvDeregisterSession(
    /* [in] */ handle_t BindingHandle,
    __RPC__in GUID SessionId
)
{
    UNREFERENCED_PARAMETER(BindingHandle);

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Enter")
    );

    std::shared_ptr<CMidiSessionTracker> sessionTracker;

    auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    RETURN_IF_FAILED(g_MidiService->GetSessionTracker(sessionTracker));

    RETURN_IF_FAILED(sessionTracker->RemoveClientSession(SessionId));

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Exit success")
    );

    return S_OK;
}

HRESULT
MidiSrvGetSessionList(
    /* [in] */ handle_t BindingHandle,
    __RPC__out BSTR* SessionListJson
)
{
    UNREFERENCED_PARAMETER(BindingHandle);

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Enter")
    );

    std::shared_ptr<CMidiSessionTracker> sessionTracker;

    auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    RETURN_IF_FAILED(g_MidiService->GetSessionTracker(sessionTracker));

    CComBSTR sessionList;
    sessionList.Empty();

    RETURN_IF_FAILED(sessionTracker->GetSessionListJson(&sessionList));

    RETURN_IF_FAILED(sessionList.CopyTo(SessionListJson));

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Exit success")
    );

    return S_OK;

}



HRESULT 
MidiSrvGetAbstractionList(
    /*[in]*/ handle_t BindingHandle,
    __RPC__out BSTR* AbstractionListJson
)
{
    UNREFERENCED_PARAMETER(BindingHandle);

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Enter")
    );

    std::shared_ptr<CMidiConfigurationManager> configManager;

    auto coInit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    RETURN_IF_FAILED(g_MidiService->GetConfigurationManager(configManager));

    auto allMetadata = configManager->GetAllEnabledTransportAbstractionLayerMetadata();

    // TODO: This code should probably live in the configuration manager instead of the RPC layer

    json::JsonObject rootObject{};

    for (auto const& metadata : allMetadata)
    {
        // add to result object

        json::JsonObject abstractionObject;
       
        if (metadata.Name != NULL) abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_NAME_PROPERTY_KEY, json::JsonValue::CreateStringValue(metadata.Name));
        if (metadata.Mnemonic != NULL) abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_MNEMONIC_PROPERTY_KEY, json::JsonValue::CreateStringValue(metadata.Mnemonic));
        if (metadata.Description != NULL) abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_DESCRIPTION_PROPERTY_KEY, json::JsonValue::CreateStringValue(metadata.Description));
        if (metadata.Author != NULL) abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_AUTHOR_PROPERTY_KEY, json::JsonValue::CreateStringValue(metadata.Author));
        if (metadata.SmallImagePath != NULL) abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_SMALL_IMAGE_PATH_PROPERTY_KEY, json::JsonValue::CreateStringValue(metadata.SmallImagePath));
        if (metadata.Version != NULL) abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_VERSION_PROPERTY_KEY, json::JsonValue::CreateStringValue(metadata.Version));
        abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_IS_RT_CREATABLE_APPS_PROPERTY_KEY, json::JsonValue::CreateBooleanValue(metadata.IsRuntimeCreatableByApps));
        abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_IS_RT_CREATABLE_SETTINGS_PROPERTY_KEY, json::JsonValue::CreateBooleanValue(metadata.IsRuntimeCreatableBySettings));
        abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_IS_SYSTEM_MANAGED_PROPERTY_KEY, json::JsonValue::CreateBooleanValue(metadata.IsSystemManaged));
        abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_IS_CLIENT_CONFIGURABLE_PROPERTY_KEY, json::JsonValue::CreateBooleanValue(metadata.IsClientConfigurable));
        if (metadata.ClientConfigurationAssemblyName != NULL) abstractionObject.SetNamedValue(MIDI_SERVICE_JSON_ABSTRACTION_PLUGIN_INFO_CLIENT_CONFIG_ASSEMBLY_PROPERTY_KEY, json::JsonValue::CreateStringValue(metadata.ClientConfigurationAssemblyName));

        // add the abstraction metadata to the root, using the abstraction id as the key
        rootObject.SetNamedValue(internal::GuidToString(metadata.Id).c_str(), abstractionObject);

        // We need to free all those bstrs in the struct. I hate this pattern.

        ::SysFreeString(metadata.Name);
        ::SysFreeString(metadata.Description);
        ::SysFreeString(metadata.Author);
        ::SysFreeString(metadata.SmallImagePath);
        ::SysFreeString(metadata.Version);
        ::SysFreeString(metadata.ClientConfigurationAssemblyName);

    }

    // stringify json to output parameter

    internal::JsonStringifyObjectToOutParam(rootObject, &AbstractionListJson);

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Exit success")
    );

    return S_OK;
}

HRESULT
MidiSrvGetTransformList(
    /*[in]*/ handle_t BindingHandle,
    __RPC__out BSTR* TransformListJson
)
{
    UNREFERENCED_PARAMETER(BindingHandle);
    UNREFERENCED_PARAMETER(TransformListJson);

    TraceLoggingWrite(
        MidiSrvTelemetryProvider::Provider(),
        __FUNCTION__,
        TraceLoggingLevel(WINEVENT_LEVEL_INFO),
        TraceLoggingWideString(L"Enter")
    );

    // TODO

    return S_OK;
}