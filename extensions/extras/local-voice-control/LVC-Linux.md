# Building an Alexa Auto SDK Linux Application with Local Voice Control (LVC)

**Table of Contents**

* [Building the Alexa Auto SDK with LVC for Linux](#building-sdk-for-linux)
* [Getting Started with LVC Using the C++ Sample App](#using-sample-app)
* [Troubleshooting](#troubleshooting)

## Building the Alexa Auto SDK with LVC for Linux<a id="building-sdk-for-linux"></a>

### Setting up the LVC Extension
Building Auto SDK with Local Voice Control functionality requires the LVC extension. The extension contains a prebuilt archive `local-voice-engine-[version].tar.gz` that provides the LVC libraries and headers required for Auto SDK to communicate with the Local Voice Control package provided separately. 
Unzip the `local-voice-control-[version].zip` archive into the `${AAC_SDK_HOME}/extensions/extras` directory:
  
```
unzip local-voice-control-2.0.zip -d ${AAC_SDK_HOME}/extensions/extras
```

> **Note**: If the archive is already unzipped, move the `local-voice-control` directory directly into the `aac-sdk/extensions/extras` directory.

> **Note**: Do not extract the local-voice-engine archive.

### Building the Alexa Auto SDK with the LVC Extension

Use the Alexa Auto SDK builder script to build the Auto SDK with the LVC extension. The currently supported Linux targets are `native`, `pokyarm`, and `pokyarm64`.

To build for native Linux, for example, use the following command:

```
builder/build.sh linux -t native extensions/extras/local-voice-control
```

Confirm that the Alexa Auto SDK Builder has added the extension by looking for the following lines in the initial console output:

```
NOTE: Search for modules & meta layers: /home/builder/aac/extensions/extras/local-voice-control
NOTE: Adding meta layer: /home/builder/aac/extensions/extras/local-voice-control/meta-aac-local-voice-control
NOTE: Search for modules: /home/builder/aac/extensions/extras/local-voice-control/modules
```

The builder command builds both the Auto SDK native libraries and the C++ Sample App. The resulting archive will be located in: `${AAC_SDK_HOME}/builder/deploy/native/aac-sdk-build-native.tar.gz`.

## Getting Started with LVC Using the C++ Sample App<a id="using-sample-app"></a>

### Installing the Sample App<a id="building-sample-app"></a>

Install the generated C++ Sample App package into `/opt/AAC`:

```shell
$ sudo tar xf ${AAC_SDK_HOME}/builder/deploy/native/aac-sdk-build-native.tar.gz -C /
```

### Installing the Local Voice Control Package

Local Voice Control functionality is provided as a self-extracting script `LVC.sh` included in the `LVC-[version].tar.gz` archive. You must untar the archive corresponding to the version of the Auto SDK that you are using to extract the `LVC.sh` script.
Use this script as follows:

```shell
Usage: LVC.sh -t <target>
    Install LVC for the target architecture under /opt/LVC/.

    -t  Target architecture (armv7ahf|aarch64|x86_64)
```

#### Custom Volume Range in LVC

If your application requires a different VUI volume range than the default (0-10, with device volume increment 10), then you must override the default values in `skillConfiguration` node of `com.amazon.alexahybrid.VolumeControlSkill` in the LVC configuration `lvc-config.json` file located at `/opt/LVC/config` after you run the installation script. Below is the `skillConfiguration` you must override for the local volume skill, and the values you set must match the custom volume settings you have configured for the cloud.

* `skillConfiguration.minVolumeValue`: The minimum volume to target by VUI
* `skillConfiguration.maxVolumeValue`: The maximum volume to target by VUI
* `skillConfiguration.volumeAdjustmentStepValue`: The volume increment with respect to device volume. This refers to the change notified via @c AlexaSpeaker.speakerSettingsChanged(), which uses the 0-100 range.

For example, the following block shows the `skillConfiguration` node of `lvc-config.json` defining the custom minimum, maximum, and device step volume adjustment levels as 0, 16, and 10 respectively:

```json
{
    ...
    "skillCollections": [
        ...
        {
            "skillCollectionId": "com.amazon.alexahybrid.VolumeControlSkill",
            "type": "local",
            "details": {
                "skillLibraryPath": "libahe_volume_control_skill.so",
                "skillConfiguration": {
                    "minVolumeValue" :  "0",
                    "maxVolumeValue" :  "16",
                    "volumeAdjustmentStepValue" : "10"
                }
            }
        }
       ...
    ],
    ...
}

```

> **Note**: This configuration applies only to the LVC VUI volume range; to set the cloud VUI volume range you need to contact your SA or Partner Manager.

### Using Local Voice Control

You can use the sample scripts from the provided archive to get started with using Local Voice Control services. To launch all services used by LVC, use the `start.sh` script:

```shell
/opt/LVC/start.sh
```

To stop Local Voice Control services, use the `stop.sh` script:

```shell
/opt/LVC/stop.sh
```

There are additional scripts to launch the LVC services individually: 

* `start-lvc.sh` starts `AlexaHybridExecutionController`
* `start-contacts-service.sh` starts `AlexaHybridContactsService`
* `start-led-service.sh` starts `DiscoveryServiceExecutable`

The included scripts are for demonstration and testing purposes only. A proper integration into your Linux platform requires you to run the different LVC services as background processes.

>**Note:** The Local Voice Control package does not provide a watchdog service, so you must configure a watchdog to spawn, monitor, and restart Local Voice Control processes as needed in your official integration. You must initialize all the required processes during system bootup, and these processes should remain running at all times except when the head unit shuts down or Alexa functionality is disabled.

### Configuring the C++ Sample App

Use the included [config-local-voice-control.json](modules/local-voice-control/configs/config-local-voice-control.json) file to pass the Local Voice Control configuration to the [C++ Sample App](../../../samples/cpp/README.md).

>**Note:** The Alexa Auto SDK uses Unix domain sockets (UDS) for inter-process communication (IPC) between components running different processes. These sockets are secured using file permissions, which should be set to be as restrictive as possible. By default the Auto SDK file permissions are set to "user only", meaning that the sockets are created to be accessible by only the Linux user running the application. In order for the Auto SDK to communicate with the LVC services running in separate processes, the same Linux user must be running both the Auto SDK and the LVC services.
 

```json
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
```

>**IMPORTANT!** To ensure successful communication between the Sample App and Local Voice Control, we recommend that you do not modify this configuration. If you do customize the directory paths for the values shown in this configuration, you must make corresponding changes to the Local Voice Control package configuration located under the installation path `/opt/LVC/config/`. 

### Starting the C++ Sample App

>**Note:** Prior to running the Sample App, ensure that the Local Voice Control package has been installed.

You must specify the [config-local-voice-control.json](modules/local-voice-control/configs/config-local-voice-control.json) file when launching the Sample App. By default that file is installed under `/opt/AAC/etc`:

```shell
$ /opt/AAC/bin/SampleApp --cbreak \ 
                         --config /opt/AAC/etc/config.json \
                         --config /opt/AAC/etc/config-local-voice-control.json \
                         --menu   /opt/AAC/etc/menu.json > SampleApp.log
```


### Using the Sample App

When you first launch the C++ Sample App on your device, please ensure the Local Voice Control services are running. It will take a moment for LVC to connect and for Alexa to become available. If you have launched the app offline or have launched it online but haven't logged in to your Amazon account, the `Connection Status` field will show `CONNECTED` when LVC has started.

>**Note:** If you are online and have logged in to your Amazon account, a cloud connection will also change the `Connection Status` to `CONNECTED`.

When you are online and logged in, it is not visually obvious whether the LVC service or the cloud service is being used. This is intentional, as the Alexa connection state is an aggregate indicator of connection to any Alexa service. `CONNECTED` means connected to at least one Alexa service and ready to accept an utterance. Alexa's responses to user utterances will make the distinction between the cloud service and the local service clear.

If you wish to force the Sample App to use the LVC connection so you can get acquainted with how Alexa responds offline, you can do one of the following:

* Log out of the application. Being `CONNECTED` and not logged in ensures you are using the local Alexa service instead of the cloud service because the cloud services aren't reachable when you're logged out.
* Disable the internet connection on the device (cellular data and/or Wi-Fi). In this case, re-enabling the internet connection will cause the Sample App to automatically resume using cloud services if you were previously logged in.

While connected to offline Alexa, you may explore using the offline features for car control, music (e.g. controlling local media sources like "CD Player"), comms (e.g. calling phone numbers or uploaded contacts), and navigation (e.g. navigating to uploaded favorite addresses or cancelling ongoing navigation).

## Troubleshooting <a id = "troubleshooting"></a>

* If the Sample App fails to start with the message `Local Voice Control is enabled but no configuration was found`:
    * Ensure that you specified the [config-local-voice-control.json](modules/local-voice-control/configs/config-local-voice-control.json) file when you launched the Sample App.
    * Ensure that the Local Voice Control package is installed and launched. The Local Voice Control package must be installed prior to launching the Sample App.
* `version CXXABI_1.3.11' not found` and `version GLIBCXX_3.4.22' not found` errors may be returned When you run `start-contacts-service.sh ` to start the Contacts service. If this occurs, run the following commands to install the required packages externally:

 ```
sudo apt-get install libstdc++6
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
```
Then run the `start-contacts-service.sh ` command again.