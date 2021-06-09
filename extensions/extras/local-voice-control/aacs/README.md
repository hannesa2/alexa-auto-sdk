# LocalVoiceControl with Alexa Auto Client Service (AACS)
The AACS configuration for the LVC module is **optional**. You can rely on AACS to configure the required fields for LVC APK, LocalVoiceControl, and LocalSkillService configurations. Use the AACS configuration for LVC only if you want to configure an optional field named `CustomVolume`. 

## Background
For the LVC APK to be configured without AACS, the LVC APK requires the following configuration. 
~~~
{
    "LocalSkillService": {
        "UnixDomainSocketPath": "/some/path/for/socket"
    },
    "CarControl": { 
        "CustomAssetsFilePath": "/some/asset/path/file.json"
    },
    "CustomVolume": {
        "minVolumeValue": "<Min VUI Volume Value>",
        "maxVolumeValue": "<Max VUI Volume Value>",
        "volumeAdjustmentStepValue": "<Device volume adjustment step value>"
    }
}
~~~

Upon receiving this configuration, the LVC APK provides a response that configures the LVC module of the Auto SDK by using the `aace.localVoiceControl` and `aace.localSkillService` fields. 
~~~
{
    "aace.localVoiceControl": {
        "controllerSocketDirectory"    : "/opt/LVC/data/ipc",
        "controllerSocketPermissions"  : "OWNER",
        "platformSocketDirectory"      : "/opt/LVC/data/ipc",
        "platformSocketPermissions"    : "OWNER",
        "address"                      : "127.0.0.1",
        "messageRouterSocketDirectory" : "/opt/LVC/data/ipc"
    },
    "aace.localSkillService": {
        "lssSocketPath": "/opt/AAC/data/lss.socket"
    }
}
~~~

## AACS Configuration
AACS configuration with LVC only requires configuration for the LVC APK, as AACS will internally process the LVC APK's response  to configure `aace.localVoiceControl` and `aace.localSkillService`. This field is called `aacs.localVoiceControl` and is shown below. 
~~~
{
    "aacs.localVoiceControl": {
        "CustomVolume": {
            "minVolumeValue": "<Min VUI Volume Value>",
            "maxVolumeValue": "<Max VUI Volume Value>",
            "volumeAdjustmentStepValue": "<Device volume adjustment step value>"
        }
    }
}
~~~

Your application no longer needs to provide the `LocalSkillService` field, since this must be a file path pointing to AACS's internal storage. As a result, AACS will handle providing the appropriate socket path here. Next, the optional `CarControl` field should not be configured using the configuration message, and your application should instead directly share permissions to the `CustomAssetsFilePath` using AACS's file sharing protocol. This will allow AACS to create a local copy of your application's car control assets file and use the path to the local copy for configuring the LVC APK. More information on how to setup FileProvider and send file sharing intents to AACS can be found in the [AACS README](../../../../platforms/android/alexa-auto-client-service/README.md). Note that the `CarControl` field is already optional, so sending a file sharing intent is only needed if you wish to configure this field. 

If you use the AACS configuration for LVC, only the `CustomVolume` field of the original LVC APK configuration remains. Because CustomVolume is optional, `aacs.localVoiceControl` is not a requirement for enabling LVC.

## LocalSearchProvider Platform Interface
LVC includes an optional platform interface `LocalSearchProvider` that when registered enables the Offline POI feature. AACS handles enabling and disabling `LocalSearchProvider` with a configuration field `useDefaultLocalSearchProvider` within `aacs.defaultPlatformHandlers`. When set to `true`, AACS will not send messages for `LocalSearchProvider` and functionality for Offline POI will not be available. To gain this functionality, set the `useDefaultLocalSearchProvider` field to `false`. Below is an example of how to configure this field to **enable** `LocalSearchProvider` AACS messages. 
~~~
{
    "aacs.defaultPlatformHandlers": {
        "useDefaultLocalSearchProvider": false,
        ...
    }
    ...
}
~~~

**Note:** Offline POI requires additional configuration to be sent to the LVC APK. AACS will handle configuring these fields. They do not have to be configured by your application. 
