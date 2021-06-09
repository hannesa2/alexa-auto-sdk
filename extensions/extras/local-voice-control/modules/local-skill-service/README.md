# Local Skill Service (LSS)


In the cloud, speechlets and skills have access to online services necessary to fulfill user requests. However, these services are not available when there is no internet connection. To serve the user offline, local skills such as car control or navigation use the Local Skill Service (LSS). The Local Skill Service provides a unified entrypoint for local skills to access the local services they need.

The Local Skill Service does not expose any platform interfaces for your Alexa Auto SDK application to implement, but it does require you to configure it, as described below.


## Configuring the Local Skill Service

The LSS module exposes handlers through an HTTP interface. The server is configured with a Unix Domain Socket, and you must configure the path to this socket. The JSON format of the configuration required by the Auto SDK Engine to configure LSS is as follows:

```
{
    "aace.localSkillService": {
        "lssSocketPath": "/opt/AAC/data/lss.socket"
    }
}
```
* The `lssSocketPath` value is the path for the Unix Domain Socket of the LSS HTTP server used by the local skills and skill services to access platform data.

The configuration you provide should match the values in the [default configuration sample file](../local-voice-control/configs/config-local-voice-control.json) because the contents of this file match the configuration used in the Local Voice Control package. Since the configuration is static, it is recommended that you use a file-based `EngineConfiguration` containing this JSON object. 

You can alternatively generate the required `EngineConfiguration` object including this JSON programmatically using the `LocalSkillServiceConfiguration::createLocalSkillServiceConfig()` factory method, if desired:

```c++
// Auto SDK Engine configuration
std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configuration;
// add other Engine configurations

...

auto config
= aace::localSkillService::config::LocalSkillServiceConfiguration::createLocalSkillServiceConfig( "/opt/AAC/data/lss.socket" );
configuration.push_back(config);
```
