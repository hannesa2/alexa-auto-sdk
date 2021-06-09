# AmazonLite Wake Word Extension

The Alexa Auto SDK AmazonLite Wake Word extension adds AmazonLite wake word support for Generic Linux (`native`), Android ARMv7a / ARMv8a / x86-64 (`androidarm` / `androidarm64` / `androidx86-64`), and QNX AArch64 / x86-64 (`qnx7arm64` / `qnx7x86-64`) targets.

> **Note**: You must use this extension with the Alexa Auto SDK Builder.

**Table of Contents**

* [Runtime Switching of the Wake Word Model](#runtime-switching-of-the-wake-word-model)
* [Installing the AmazonLite Wake Word Extension](#installing-the-amazonlite-wake-word-extension)
* [Building the Auto SDK with the AmazonLite Wake Word Extension](#building-the-auto-sdk-with-the-amazonlite-wake-word-extension)
* [Running the Auto SDK with the AmazonLite Wake Word Extension](#running-the-auto-sdk-with-the-amazonlite-wake-word-extension)

## Runtime Switching of the Wake Word Model<a id = "runtime-switching-of-the-wake-word-model"></a>

AmazonLite optionally supports switching the wake word model when the AVS locale is switched. Enabling this feature requires [access to the binary format locale models on the target device](#binary-format-locale-access) and [AmazonLite configuration](#amazonlite-configuration).

### Access to the Binary Format Locale Models on Target Device <a id = "binary-format-locale-access"></a>

AmazonLite requires locale models in binary format on the target file system to support the runtime switching feature. These model files are part of the build artifact. You can find them at: `/opt/AAC/share/pryon-lite/models` inside the build artifact `aac-sdk-build-<target>.tar.gz` located at `builder/deploy/<target>`.

### Providing AmazonLite Configuration<a id = "amazonlite-configuration"></a>

In addition to access to the locale models, the application must provide the `aace.amazonLite` configuration, which specifies the path where model files are stored and maps the wake word locale to these model files.

To programmatically pass the `aace.amazonLite` configuration see [aace::engine::amazonLite::AmazonLiteConfiguration](./modules/amazonlite/engine/include/AACE/Engine/AmazonLiteConfiguration.h) class for more details. Or supply the configuration as outlined below.

```
"aace.amazonLite": {
    "rootPath": "/path/to/models",
    "models": [
        {
            "locale": "en-US",
            "path": "D.en-US.alexa.bin"
        },
        {
            "locale": "en-IN",
            "path": "D.en-IN.alexa.bin"
        }
        ...
    ]
    },
```


> **Note**: For the most accurate internationalized wake word detection, we recommend that you have the wake word locale model for each supported Alexa locale. If a matching locale is not found/supplied then the AmazonLite Wake Word extension falls back to the default locale.

> **Note**: Optionally you may refer to `builder/deploy/[target]/opt/AAC/share/pryon_lite/models/localeToModels.json` for details on mapping locales to the model files.

## Installing the AmazonLite Wake Word Extension<a id = "installing-the-amazonlite-wake-word-extension"></a>

Unzip the `amazonlite-2.2.zip` archive into the `${AAC_SDK_HOME}/extensions/extras` directory. For example:

```
unzip amazonlite-2.2.zip -d aac-sdk/extensions/extras
```

> **Note**: If the archive is already unzipped, simply move the `amazonlite` directory into the `aac-sdk/extensions/extras` directory.

Your `aac-sdk/extensions/extras` directory structure should look something like this:

```
aac-sdk
└─ extensions
   └─ extras
      └─ amazonlite
         └─ meta-aac-amazonlite
            └─ conf
               └─ layer.conf
            └─ packages
               ├─ pryon_lite_[version]-alexa-autocore-[date and build].zip
               └─ avs-sdk-adapter-for-wakeword-lite-[version].tgz
         └─ modules
           └─ amazonlite
              └─ configs
                 └─ config-amazonlite.json.in
           └─ engine
           └─ platform
         └─ platforms
            └─ android
         └─ LICENSE
         └─ README.md
```

## Building the Auto SDK with the AmazonLite Wake Word Extension <a id="building-the-auto-sdk-with-the-amazonlite-wake-word-extension"></a>

To enable wake word support with the default locale, build your target platform with the AmazonLite Wake Word extension. See below for build instructions for [Android](#building-for-android), [Linux](#building-for-linux), and [QNX
](#building-for-qnx)
### Overriding the Default Wake Word Model

As a default option, AmazonLite is configured to a default wake word locale `en-US`.

To override this default locale, set the BitBake flag `AMAZONLITE_CPP_MODEL_NAME` to the desired locale model CPP file as found inside the `amazonlite/meta-aac-amazonlite/packages/pryon_lite-*/models/[format]`

For example, to override the default wake word model to `en-IN`:

```
builder/build.sh <platform> --target <target> extensions/extras/amazonlite -DAMAZONLITE_CPP_MODEL_NAME=D.en-IN.alexa.cpp
```

### Building the Auto SDK with the AmazonLite Wake Word Extension for Android <a id = "building-for-android"></a>


Build your Android target with the `amazonlite` extension to enable wake word support.

For example:

```
builder/build.sh android --target androidarm,androidx86-64 extensions/extras/amazonlite
```
Confirm that the Alexa Auto SDK Builder has added the extension by looking for the following lines in the initial console output:

```
NOTE: SDK Version: [version]
NOTE: Search for sub modules: /home/builder/aac/extensions/extras/amazonlite
NOTE: Adding meta layer: /home/builder/aac/extensions/extras/amazonlite/meta-aac-amazonlite
```

#### Enabling Wake Word Model Switching
Complete the following steps if you want to enable an Android Sample App feature to enable wake word model switching:

1. Untar `aac-sdk-build-<target>.tar.gz` found at `builder/deploy/` and copy the required locale model binary files `(*.bin)` from `builder/deploy/[target]/opt/AAC/share/pryon_lite/models` to `${AAC_SDK_HOME}/samples/android/app/src/main/assets/models`.

2. Provide the `aace.amazonLite` AmazonLite configuration to the Engine by uncommenting this line of code in [`AmazonliteModuleFactory.java`](./samples/android/modules/sample-amazonlite/src/main/java/com/amazon/sampleapp/amazonlite/AmazonliteModuleFactory.java):

  ```
Configuration.createAmazonLiteConfig(sampleAppContext.getData(SampleAppContext.MODEL_DIR), modelsInfoList));
```

3. Rebuild the Android target: 

  ```
builder/build.sh android --target androidarm,androidx86-64 extensions/extras/amazonlite
```

### Building the Auto SDK with the AmazonLite Wake Word Extension  for Linux<a id="building-for-linux"></a>

Build your Linux target with the wake word extension to enable wake word support.

For example:

```
builder/build.sh linux -t native extensions/extras/amazonlite
```

Confirm that the Alexa Auto SDK Builder has added the extension by looking for the following lines in the initial console output:

```
NOTE: SDK Version: [version]
NOTE: Search for sub modules: /home/builder/aac/extensions/extras/amazonlite
NOTE: Adding meta layer: /home/builder/aac/extensions/extras/amazonlite/meta-aac-amazonlite
```

#### Building the C++ Sample App with the AmazonLite Wake Word Extension

Build the [C++ Sample App](../../samples/cpp/README.md) with the wake word extension to enable wake word support with the default locale.

```
builder/build.sh linux -t native extensions/extras/amazonlite
```

Install the generated C++ Sample App package into `/opt/AAC`:

```shell
$ sudo tar xf ${AAC_SDK_HOME}/builder/deploy/native/aac-sdk-build-native.tar.gz -C /
```

Run the [C++ Sample App](../../samples/cpp/README.md).

```shell
$ /opt/AAC/bin/SampleApp --cbreak \
                         --config /opt/AAC/etc/config.json \
                         --menu   /opt/AAC/etc/menu.json 
```

##### Enabling Wake Word Model Switching

Configure the [C++ Sample App](../../samples/cpp/README.md) with the `aace.amazonLite` config to enable run time switching of the wake word model. There are two ways to provide the config and run the Sample App:

###### Option 1: Use the autogenerated config 

This is the easiest option, as it uses the config from `/opt/AAC/etc/config-amazonlite.json` which is generated at the build time using the [config-amazonlite.json.in](./modules/amazonlite/configs/config-amazonlite.json.in) with default config values. Models here are assumed to be stored in the default `/opt/AAC/share/pryon-lite/models` location.

Use these commands to run the C++ Sample App with the autogenerated AmazonLite config:

```shell
$ /opt/AAC/bin/SampleApp --cbreak \
                         --config /opt/AAC/etc/config.json \
                         --config /opt/AAC/etc/config-amazonlite.json \
                         --menu   /opt/AAC/etc/menu.json > SampleApp.log
```

###### Option 2: Create a custom config file

Alternatively you can provide the `aace.amazonLite` config by creating your own config file and passing it in when running the SampleApp. Copy the [config-amazonlite.json.in](./modules/amazonlite/configs/config-amazonlite.json.in) file, rename it as `config-amazonlite.json`, and customize the config values. The config you create must follow the same JSON format as the [config-amazonlite.json.in](./modules/amazonlite/configs/config-amazonlite.json.in) file. For example:

```
{
    "aace.amazonLite": {
        "rootPath": "/opt/AAC/share/pryon-lite/models",
        "models": [
            {
                "locale": "en-US",
                "path": "D.en-US.alexa.bin"
            },
            ...
        ]
    }
}
```

> **Note:**  Update the `rootPath` with the appropriate absolute path where the model files are stored. The above example reads the models from `/opt/AAC/share/pryon-lite/models`.

Run the [C++ Sample App](../../samples/cpp/README.md) with the absolute path to the `config-amazonlite.json` file.

> **NOTE** Before running the command update the `/path/to/config-amazonlite.json` to appropriate path where `config-amazonlite.json` is located.

```shell
$ /opt/AAC/bin/SampleApp --cbreak \
                         --config /opt/AAC/etc/config.json \
                         --config /path/to/config-amazonlite.json \
                         --menu   /opt/AAC/etc/menu.json > SampleApp.log
```


### Building the Auto SDK with the AmazonLite Wake Word Extension for QNX<a id = "building-for-qnx"></a>

Build your QNX target with the AmazonLite Wake Word extension to enable wake word support.

For example:

```
builder/build.sh qnx7 -t qnx7arm64 --qnx7sdp-path ${HOME}/qnx700 extensions/extras/amazonlite
```

Confirm that the Alexa Auto SDK Builder has added the extension by looking for the following lines in the initial console output:

```
NOTE: SDK Version: [version]
NOTE: Search for sub modules: /home/builder/aac/extensions/extras/amazonlite
NOTE: Adding meta layer: /home/builder/aac/extensions/extras/amazonlite/meta-aac-amazonlite
```

## Running the Auto SDK with the AmazonLite Wake Word Extension<a id ="running-the-auto-sdk-with-the-amazonlite-wake-word-extension"></a>

### Running the Auto SDK with the AmazonLite Wake Word Extension on Android

If the Android Sample App detects at launch that the SDK was built with wake word support, a Wake Word switch will appear in its control drawer, set `on` by default.

Open the Sample App in Android Studio, then run the app on your target device, log in, and start using the wake word: **"Alexa... What time is it?"**

In your implementation, your `SpeechRecognizerHandler` should start streaming microphone data with `startAudioInput()`, and stop with `stopAudioInput()`.

For example:

```
public class SpeechRecognizerHandler extends SpeechRecognizer
{
    @Override
    public boolean startAudioInput() {
        // implement start recording
    }
    @Override
    public boolean stopAudioInput() {
        // implement stop recording
    }
    ...
        // call the write audio method while reading audio input data
        write( (byte[]) data, (long) size )
    ...
}
```

### Running the Auto SDK with the AmazonLite Wake Word Extension on Linux or QNX

The `alexa` module detects wake word support, which can be enabled/disabled at instance creation, or later through `SpeechRecognizer` calls `enableWakewordDetection()` and `disableWakewordDetection()`.

You can also support enabling/disabling with an optional `wakewordDetectionEnabled` parameter in your `SpeechRecognizerHandler` constructor.

For example:

```
SpeechRecognizerHandler::SpeechRecognizerHandler( bool wakewordDetectionEnabled = true )
    : aace::alexa::SpeechRecognizer{ wakewordDetectionEnabled } { ... }
```

Additionally, your `SpeechRecognizerHandler` implementation should start streaming microphone data with `startAudioInput()`, and stop with `stopAudioInput()`.

For example:

```
bool SpeechRecognizerHandler::startAudioInput() {
    auto audioInput = m_audioInput.lock();
    if (!audioInput) {
        return false;
    }
    return audioInput->startStreamingMicrophoneData();
}

bool SpeechRecognizerHandler::stopAudioInput() {
    auto audioInput = m_audioInput.lock();
    if (!audioInput) {
        return false;
    }
    return audioInput->stopStreamingMicrophoneData();
}
```

The audio input writes microphone data as it becomes available (e.g. audio input callback) to the `SpeechRecognizerHandler` instance.

For example:

```
ssize_t returnCode = speechRecognizerHandler->write(static_cast<const int16_t *>(inputBuffer), numSamples);
```
