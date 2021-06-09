# Building an Alexa Auto SDK Android Application with Local Voice Control

**Table of Contents**

* [Building the Alexa Auto SDK with LVC for Android](#building-sdk-for-android)
* [Understanding the LVC APK](#understanding-lvc-apk)
* [Getting Started with LVC using the Android Sample App](#using-sample-app)
* [Building your own Alexa Auto Application with LVC](#building-custom-app)
* [Signing an APK](#signing-apk)
* [Troubleshooting](#troubleshooting)

## Building the Alexa Auto SDK with LVC for Android<a id="building-sdk-for-android"></a>

### Setting up the LVC module
Building the Alexa Auto SDK with Local Voice Control (LVC) functionality requires the LVC extension. The extension contains a prebuilt archive `local-voice-engine-[version].tar.gz` that provides the LVC libraries and headers required for the Auto SDK to use the Local Voice Control module included in the extension. 
Unzip the `local-voice-control-[version].zip` archive into the `${AAC_SDK_HOME}/extensions/extras` directory:
  
```
unzip local-voice-control-2.0.zip -d ${AAC_SDK_HOME}/extensions/extras
```

> **Note**: If the archive is already unzipped, move the `local-voice-control` directory directly into the `aac-sdk/extensions/extras` directory.

> **Note**: Do not extract the local-voice-engine archive.

### Building the Alexa Auto SDK with the LVC extension

Use the Alexa Auto SDK builder script to build the Auto SDK with the LVC extension. The currently supported Android targets are `androidx86-64`, `androidarm`, and `androidarm64`.

To build for Android x86 64-bit, for example, use the following command:

```
builder/build.sh android -t androidx86-64 extensions/extras/local-voice-control
```

Confirm that the Alexa Auto SDK Builder has added the extension by looking for the following lines in the initial console output:

```
NOTE: Search for modules & meta layers: /home/builder/aac/extensions/extras/local-voice-control
NOTE: Adding meta layer: /home/builder/aac/extensions/extras/local-voice-control/meta-aac-local-voice-control
NOTE: Search for modules: /home/builder/aac/extensions/extras/local-voice-control/modules
```

The builder command builds both the Auto SDK native libraries and the AARs that package the libraries for Android. The AARs will be located in `${AAC_SDK_HOME}/aac-sdk/builder/deploy/aar` and will include the standard Auto SDK AARs as well as those from the LVC extension.

## Understanding the LVC APK <a id="understanding-lvc-apk"></a>
LVC components with which the Auto SDK Engine interacts are packaged and delivered as an unsigned APK (`lveapp-release.apk`), hereafter referred to as the "LVC APK". The components of the LVC APK interact with an application running the Auto SDK Engine via inter-process communication (IPC) using Unix Domain Sockets (UDS).

The UDS sockets are secured using file permissions that are as restrictive as possible. By default, Auto SDK and LVC APK permissions are set to "user only", meaning the sockets are created to be accessible by only the user running the application. In order for the Auto SDK application and the LVC APK sockets to communicate and for the two applications to share data, both must run under the same Linux user.

The LVC APK contains a speech recognition and synthesis engine that uses separate models for each supported language. Beginning with Auto SDK v2.2.0, the language models have been separated into independent APKs, rather than being provided directly in the LVC APK as they were in previous releases. One language model APK is provided for each supported locale (currently en-US, en-CA, and fr-CA).

## Getting Started with LVC using the Android Sample App<a id="using-sample-app"></a>

If you want to use the Sample App build in online mode as well, edit the `app_config.json` file normally as described in `${AAC_SDK_HOME}/samples/android/README.md`.

Your Sample App APK must be signed with a key from your (new or existing) keystore in order to use Local Voice Control. You can build an unsigned APK using either command line Gradle or Android Studio and then [sign it](#signing-apk), or you can build a signed APK directly using Android Studio.

<details>
<summary>Building an unsigned Sample App APK with command line Gradle</summary>

To build an unsigned APK with command line Gradle:

1.  Install `gradlew` (if it is not already installed in `${AAC_SDK_HOME}/samples/android/`). From the `${AAC_SDK_HOME}/samples/android/` directory, run the following command:

    `gradle wrapper --gradle-version <gradle_version_number>`
    
    > **Note**: If `gradlew` is not available in your project, open the project in Android Studio and allow gradle wrapper installation.  
2. Generate an unsigned APK using `gradlew` from `${AAC_SDK_HOME}/samples/android`:

 ```
$ gradle assembleLocalRelease
```
 > **Note**: Use `assembleLocalDebug` to generate debug builds.

The generated Alexa Auto Sample App release APK will be located under `${AAC_SDK_HOME}/samples/android/app/build/outputs/apk/release/`. You can now  [sign the Sample App APK](#signing-apk).
</details>
    
<details>
<summary>Building an unsigned Sample App APK with Android Studio</summary>

To build an unsigned APK with Android Studio:

1. Launch <a href=https://developer.android.com/studio/index.html>Android Studio</a> and select **Open an existing Android Studio project**.

2. Open the `${AAC_SDK_HOME}/samples/android` folder and click the <strong>Open</strong> button. (Tested with Android Studio version 3.x)
3. Populate the `app_config.json` file in the assets directory with the Client ID and Product ID for your product. The Product DSN may be any unique identifier, and you can add it with the key "productDsn". By default this is the device specific ANDROID\_ID. The contents of this file are required for authorization.
4. Under **Build Variants**, select **localRelease**.
5. Click the **Run** button (green triangle) to build and deploy the app to your selected target device.

You can now follow the instructions to [sign the Sample App APK](#signing-apk).
</details>

<details>
<summary>Building a signed APK directly from Android Studio</summary>

1. [Configure Android Studio with your keystore](https://developer.android.com/studio/publish/app-signing#generate-key).
2. [Build the signed APK from Android Studio](https://developer.android.com/studio/publish/app-signing#sign_release).
3. Install the signed APK on your target device.
</details>

### Installing the LVC APK and the Language Model APKs<a id="installing-signing-lvc-apk"></a>

Because the Sample App and the LVC APK must run under the same Linux user, both applications must be signed with the same key. To sign the LVC APK, follow the instructions under [Signing an APK](#signing-apk). Once you have signed it, install the signed LVC APK on your target device.

Because the Sample App and the LVC APK must run under the same Linux user, both applications must be signed with the same key. In addition, you must sign the language model APKs with the same key to ensure that the LVC APK uses the correct language models. Once you have signed the APKs, install the signed LVC and language model APKs on your target device.

### Using the Sample App

When you first launch the Android Sample App on your device, the Local Voice Control service will launch and start automatically. It will take a moment for LVC to connect and for Alexa to become available. If you have launched the app offline or have launched it online but haven't logged in to your Amazon account, the `Connection Status` field in the control drawer at the right side of the Sample App will show `CONNECTED` when LVC has started.

>**Note:** If you are online and have logged in to your Amazon account, a cloud connection will also change the `Connection Status` to `CONNECTED`.

When you are online and logged in, it is not visually obvious whether the LVC service or the cloud service is being used. This is intentional, as the Alexa connection state is an aggregate indicator of connection to any Alexa service. `CONNECTED` means connected to something (either the cloud service or the local service) and ready to accept an utterance. Alexa's responses to user utterances will make the distinction between the cloud service and the local service clear.

If you want to force the Sample App to use the LVC connection so you can get acquainted with how Alexa responds offline, you can do one of the following:

* Log out from the control drawer at the right side of the Sample App. Being `CONNECTED` and not logged in ensures you are using the local Alexa service instead of the cloud service because the cloud services aren't reachable when you're logged out from your Amazon account.
* Disable the internet connection on the device (cellular data and/or Wi-Fi). In this case, re-enabling the internet connection will cause the Sample App to automatically resume using cloud services if you were previously logged in.

While connected to offline Alexa, you may explore using the offline features for car control, music (e.g. controlling local media sources like "CD Player"), comms (e.g. calling phone numbers or uploaded contacts), and navigation (e.g. navigating to uploaded favorite addresses or cancelling ongoing navigation).

## Building your own Alexa Auto Application with LVC<a id="building-custom-app"></a>

The LVC APK that allows your Alexa Auto SDK application to use Local Voice Control functionality is an invisible-to-the-user application that provides a single Android Service with which your application communicates. Your application is responsible for launching the LVC service, contributing configuration, starting the LVC service, and receiving configuration that you must use to configure the Auto SDK Engine. The details of implementing an application equipped to do this are described in the following subsections. An implementation of the specification that follows is provided for reference in the Auto SDK Android Sample App.

### Get the LVC APK and Language Model APKs

The LVC APK (`lveapp-release-unsigned.apk`) is an unsigned APK that allows the Alexa Auto SDK Engine hosted by your Auto SDK application to communicate with Local Voice Control components via UDS IPC. It is included in the `lveapp-release-unsigned-[version].tar.gz` archive. You must untar the version of the archive corresponding to the version of the Auto SDK you are using to extract the LVC APK, and you must sign the LVC APK with a key before installation (see [Signing an APK](#signing-apk) for instructions). You must also install the language model APK corresponding to each desired locale after signing the APKs with the same key.

### Implement your communication channel to the LVC service<a id="implment-lvc-communication"></a>

#### Set up permissions

To enable the two applications to share data and use each other's "user only" permission-secured sockets, run your application and the LVC APK under the same Linux user ID. Declare the `sharedUserId` to be `com.amazon.AlexaAutoUserId` in the manifest of your application:

```
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    android:sharedUserId="com.amazon.AlexaAutoUserId"
    package="<YOUR_ALEXA_AUTO_APP_PACKAGE_NAME>">
```
Both applications will share the same user ID as long as you sign them with the same certificate, so before you install it on your target device, you must sign your application with the same key that you used to sign the LVC APK.

The single entrypoint to the Local Voice Control functionality bundled in the LVC APK is an Android service called `LocalVoiceControlService`, hereby known as the "LVC Service". To use the LVC Service, you should declare the LVC permission defined by the LVC APK in your application's manifest:

``` xml
<uses-permission android:name="com.amazon.alexaauto.lvc" />
```

#### Get an ILVCService instance

Your application should bind to the LVC Service using an intent with action `"com.amazon.alexalve.LocalVoiceControlService"` and package `"com.amazon.alexalve"`. The `onServiceConnected()` callback of your `ServiceConnection` will return an `IBinder` with underlying type `ILVCService`:

```
private ServiceConnection mConnection = new ServiceConnection() {
    public void onServiceConnected(ComponentName className, IBinder service) {
        mLVCService = ILVCService.Stub.asInterface(service);
        // ILVCService instance mLVCService will be used to talk to the LVC service
        // ...
    }
}
```
`ILVCService` and `ILVCClient` are AIDL interfaces defined by the LVC APK that define the IPC contract between the LVC Service and your application that uses it. The full specification for each of these interfaces is available in the Android platform assets directory in [`ILVCService.aidl`](./platforms/android/assets/ILVCService.aidl) and [`ILVCClient.aidl`](./platforms/android/assets/ILVCClient.aidl). Your application will need a copy of these files in its codebase in order to use these interfaces.

#### Register an ILVCClient implementation

Your application must define a component that implements the `ILVCClient` contract and register an instance of that component with the `ILVCService` after the binding is established. 

```
// define an ILVCClient implementation
private ILVCClient mLVCClient = new ILVCClient.Stub() {
    @Override
    public String getConfiguration() {
        String configString = "";
        // ...
        return configString;
    }

    @Override
    public void configure(String configuration) {
        // ...
    }

    @Override
    public void onStart() {
        // ...
    }

    @Override
    public void onStop() {
        // ...
    }
};
```

```
// register your ILVCClient
mLVCService.registerClient(mClient);
```

#### Configure the LVC Service<a id="configuration"></a>

Registering your `ILVCClient` notifies the LVC Service that your component will provide the client configuration that the LVC service needs in order to start. Binding to the LVC Service begins its configuration sequence, but it cannot complete until your registered `ILVCClient` supplies its configuration when the LVC service asks for it with a call to `ILVCClient.getConfiguration()` after your client is registered. The configuration you return must be a string containing a JSON formatted as follows:


```
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
```

* The `LocalSkillService.UnixDomainSocketPath` value is the path you have selected to host the UDS socket of the Local Skill Service inside the Auto SDK Engine. The path you provide here must match the path you provide in the Local Skill Service configuration that you use to configure the Engine. LVC components running in the LVC APK use this socket for IPC with Auto SDK's Local Skill Service.
* The `CarControl.CustomAssetsFilePath` value defines a path to a file that defines custom assets for your Car Control implementation. See the [Car Control module documentation](./platforms/android/modules/car-control/README.md) for details about car control assets and the format of the file. If your application does not use any custom car control assets, exclude the "CarControl" node entirely from this configuration.
* The `CustomVolume` node defines configuration for a VUI volume range that is different from the default (0-10, with device volume increment 10). Supply this node only if your device uses a custom VUI volume range for online utterances as well. If used, all values in this node are required to be present and match the custom configuration you use for the cloud.

> **Note**: This `CustomVolume` applies only to the LVC VUI volume range; to set the Cloud VUI volume range you need to contact your SA or Partner Manager.

#### Start the LVC Service

After the LVC Service has completed its configuration sequence, it waits to start its internal components until the client notifies it to start with a call to `ILVCService.start()`. This is an asynchronous operation that the client must call to tell the service to start when ready. When all internal components of the LVC service have started, the client `ILVCClient.onStart()` method will be invoked by the LVC service.

#### Stop the LVC Service

The started LVC service can be shut down with a call to `ILVCService.stop()`. This is an asynchronous operation that the client can call to tell the LVC Service to stop its internal components. When all internal components have stopped, the client `ILVCClient.onStop()` method will be invoked by the LVC service. A stopped LVC service may be started again later, if needed. 

To shut down the LVC service entirely, your client should stop the service, wait for `onStop()`, unregister itself, then release its binding.

```
mLVCService.unregisterClient(mClient);
unbindService(mConnection);
```

## Signing an APK <a id = "signing-apk"></a>

To sign an APK you can follow these steps:

1. Create a custom keystore using the following command, or skip to the next step and use an existing keystore: 

    ```
    keytool -genkey -v -keystore <keystore_name>.keystore -alias <alias> -keyalg RSA -keysize 2048 -validity 10000
    ```
2. `cd` into the directory where the APK exists, for example `${AAC_SDK_HOME}/samples/android/app/build/outputs/apk/release/` for the Sample App.

3. Use `zipalign` as shown below: 
4. 
    ```
    zipalign -v -p 4 <apk_name>-unsigned.apk <apk_name>-unsigned-aligned.apk
    ```
    >**Note:** The `zipalign` tool is part of the Android SDK Build Tools, and on Mac it is usually located in `~/Library/Android/sdk/build-tools/<Android_SDK_Build_Tools_version>/zipalign`.

4. Sign the APK using your keystore using the `apksigner` command, as shown below: 
 
    ```
    apksigner sign --ks <path_to_keystore>/<keystore_name>.keystore  --ks-pass pass:<passphrase> --out <apk_name>.apk <apk_name>-unsigned-aligned.apk
    ```

6. When prompted, enter the passphrase that you set when you created the keystore. The `apksigner` tool is part of Android SDK Build Tools 24.0.3 or higher, and on Mac it is usually found in `~/Library/Android/sdk/build-tools/<Android_SDK_Build_Tools_version>/apksigner`.
7. Install the signed APK on your target device.

## Troubleshooting<a id="troubleshooting"></a>

* If you encounter a `Permission denied` error while executing `gradlew`, change the `gradlew` permissions:

   ```
   chmod +x gradlew
   ```

* If you encounter an `INSTALL_FAILED_SHARED_USER_INCOMPATIBLE` error when you try to install the LVC APK, the Sample App APK, or your own APK, first try uninstalling any existing APK that uses the `com.amazon.AlexaAutoUserId` shared user in its manifest. For example, to uninstall the Alexa Auto SDK Sample App APK and the LVC APK:

    ```
    adb uninstall com.amazon.sampleapp
    adb uninstall com.amazon.alexalve
    ```
    Then try reinstalling the APK that failed to install.