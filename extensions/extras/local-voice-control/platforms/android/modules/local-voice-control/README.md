# Local Voice Control Module

The Alexa Auto SDK Local Voice Control module implements the platform-specific support required for Local Voice Control. It is used in conjunction with the [Local Skill Service](../../modules/local-skill-service/README.md), [Address Book Local Service](../../modules/address-book-local-service/readme.md), and [Car Control](../../modules/car-control/README.md) modules as well as the standard Auto SDK modules.

 The Local Voice Control module does not expose any platform interfaces for your Alexa Auto SDK application to implement, but it does require you to configure it as described below.

## Configuring the Local Voice Control Module

The JSON format of the configuration required by the Auto SDK Engine to configure the Local Voice Control module is as follows:

```
{
    "aace.localVoiceControl": {
        "controllerSocketDirectory"    : "/path/for/ec/socket",
        "controllerSocketPermissions"  : "OWNER",
        "platformSocketDirectory"      : "/path/for/ps/socket",
        "platformSocketPermissions"    : "OWNER",
        "address"                      : "127.0.0.1",
        "messageRouterSocketDirectory" : "/path/for/mr/socket"
    }
}
```
* The `controllerSocketDirectory` value is the path where LVC will create the Unix Domain Socket it uses for its "Execution Controller".
* The `controllerSocketPermissions` value is the permission for the Execution Controller socket.
* The `platformSocketDirectory` value is the path where LVC will create the socket it uses for its "Platform Services".
* The `platformSocketPermissions` value is the permission for the Platform Services socket.
* The `address` value is the IP address of the local Alexa endpoint.
* The `messageRouterSocketDirectory` value is the path where the hybrid message router will create the socket it uses for connection with the local Alexa endpoint.

You can generate the required `EngineConfiguration` object including this JSON programmatically using the `LocalVoiceControlConfiguration.createIPCConfig()` factory method. Since the `LocalVoiceControlService` in the LVC APK provides the values to use for the socket directories and permissions and derives them from the paths it determines at runtime, it is required to use the programmatic configuration generation over providing hardcoded paths in a static configuration file. It is the responsibility of your application that implements `ILVCClient` to extract the required values from the `ILVCCLient.configure()` payload and supply them in your Engine configuration. See the following example that demonstrates which values to use as arguments of `createIPCConfig()`:

```java
// Auto SDK Engine configuration
ArrayList<EngineConfiguration> configuration = new ArrayList<EngineConfiguration>(Arrays.asList(
    // ... other Engine configurations ...
));

...

// 'json' refers to the configuration string provided by the LVC APK.
// We get the values we want to use from this configuration
JSONObject lvcConfig = new JSONObject(json);
JSONObject aheNode = lvcConfig.getJSONObject("AlexaHybridEngine");
JSONObject aheECNode = aheNode.getJSONObject("ExecutionController");

String ecSocketPath = aheECNode.getString("PlatformInterfaceSocketPath");
String ecSocketPermission = aheECNode.getString("PlatformInterfaceSocketPermissions");
String ecMRSocketPath = aheECNode.getString("MessageRouterSocketPath");

JSONObject ahePINode = aheNode.getJSONObject("PlatformInterfaces");
String psSocketPath = ahePINode.getString("UnixDomainSocketPath");
String psSocketPermission = ahePINode.getString("UnixDomainSocketPermissions");

String localAddress = "127.0.0.1";

configuration.add(LocalVoiceControlConfiguration.createIPCConfig(
        ecSocketPath,
        LocalVoiceControlConfiguration.stringToSocketPermission(ecSocketPermission),
        psSocketPath,
        LocalVoiceControlConfiguration.stringToSocketPermission(psSocketPermission),
        localAddress,
        ecMRSocketPath));
```