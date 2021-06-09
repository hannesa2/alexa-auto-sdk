# Local Skill Service (LSS)


In the cloud, speechlets and skills have access to online services necessary to fulfill user requests. However, these services are not available when there is no internet connection. To serve the user offline, local skills such as car control or navigation use the Local Skill Service (LSS). The Local Skill Service provides a unified entrypoint for local skills to access the local services they need.

The Local Skill Service does not expose any platform interfaces for your Alexa Auto SDK application to implement, but it does require you to configure it, as described below.


## Configuring the Local Skill Service

The LSS module exposes handlers through an HTTP interface. The server is configured with a Unix Domain Socket, and you must configure the path to this socket. The JSON format of the configuration required by the Auto SDK Engine to configure LSS is as follows:

```
{
    "aace.localSkillService": {
        "lssSocketPath": "/path/for/lss/socket"
        "lmbSocketPath": "/path/for/lmb/socket"
    }
}
```
* The `lssSocketPath` value is the path for the Unix Domain Socket of the LSS HTTP server used by the local skills and skill services to access platform data.
* The optional `lmbSocketPath` value is the path for the Unix Domain Socket of the Local Model Builder HTTP server. Configuring this path allows LSS to upload entities used by LVC for entity resolution of specific slot types.

You can generate the required `EngineConfiguration` object including this JSON programmatically using the `LocalVoiceControlConfiguration.createLocalSkillServiceConfig()` factory method. Since the `LocalVoiceControlService` in the LVC APK provides the value to use for `lmbSocketPath` and determines it at runtime, it is recommended to use the programmatic configuration generation over providing hardcoded paths in a static configuration file. It is the responsibility of your application that implements `ILVCClient` to extract the required values from the `ILVCCLient.configure()` payload and supply them in your Engine configuration. See the following example that demonstrates which values to use as arguments of `createLocalSkillServiceConfig()`:


```java
// Auto SDK Engine configuration
ArrayList<EngineConfiguration> configuration = new ArrayList<EngineConfiguration>(Arrays.asList(
    // ... other Engine configurations ...
));

...

// This must be the same path for the LSS socket you provided in the LVC APK configuration
// via ILVCClient.getConfiguration().
// For example,
String lssSocketPath = getCacheDir().getAbsolutePath() + "/LSS.socket";

// 'json' refers to the configuration string provided by the LVC APK.
// We get the LMB socket path we want to use from this configuration
JSONObject lvcConfig = new JSONObject(json);
JSONObject aheNode = lvcConfig.getJSONObject("AlexaHybridEngine");
JSONObject aheAMNode = aheNode.getJSONObject("ArtifactManager");
String lmbSocketPath = aheAMNode.getJSONObject("IngestionEP").getString("UnixDomainSocketPath");

configuration.add(LocalSkillServiceConfiguration.createLocalSkillServiceConfig(lssSocketPath, lmbSocketPath));
```
