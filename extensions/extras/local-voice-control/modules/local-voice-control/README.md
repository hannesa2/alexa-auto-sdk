# Local Voice Control Module

The Alexa Auto SDK Local Voice Control (LVC) module implements the platform-specific support required for Local Voice Control. It is used in conjunction with the modules provided in the LVC extension as well as the standard Auto SDK modules.

The Local Voice Control module does not expose any platform interfaces for your Alexa Auto SDK application to implement, but it does require you to configure it as described below.

## Configuring the Local Voice Control Module

The JSON format of the configuration required by the Auto SDK Engine to configure the Local Voice Control module is as follows:

```
{
    "aace.localVoiceControl": {
        "controllerSocketDirectory"    : "/opt/LVC/data/ipc",
        "controllerSocketPermissions"  : "OWNER",
        "platformSocketDirectory"      : "/opt/LVC/data/ipc",
        "platformSocketPermissions"    : "OWNER",
        "address"                      : "127.0.0.1",
        "messageRouterSocketDirectory" : "/opt/LVC/data/ipc"
    }
}
```
* The `controllerSocketDirectory` value is the path where LVC will create the Unix Domain Socket it uses for its "Execution Controller".
* The `controllerSocketPermissions` value is the permission for the Execution Controller socket.
* The `platformSocketDirectory` value is the path where LVC will create the socket it uses for its "Platform Services".
* The `platformSocketPermissions` value is the permission for the Platform Services socket.
* The `address` value is the IP address of the local Alexa endpoint.
* The `messageRouterSocketDirectory` value is the path where the hybrid message router will create the socket it uses for connection with the local Alexa endpoint.

The configuration you provide should match the values in the [default configuration sample file](./configs/config-local-voice-control.json) because the contents of this file match the configuration used in the Local Voice Control package. Since the configuration is static, it is recommended that you use a file-based `EngineConfiguration` containing this JSON object. 

You can alternatively generate the required `EngineConfiguration` object including this JSON programmatically using the `LocalSkillServiceConfiguration::createIPCConfig()` factory method, if desired:

```c++
// Auto SDK Engine configuration
std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configuration;
// add other Engine configurations

...

auto config = aace::localVoiceControl::config::LocalVoiceControlConfiguration::createIPCConfig(
    "/opt/LVC/data/ipc", 
    "OWNER", 
    "/opt/LVC/data/ipc",
    "OWNER",
    "127.0.0.1",
    "/opt/LVC/data/ipc" );
configuration.push_back(config);
```