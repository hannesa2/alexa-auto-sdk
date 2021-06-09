/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.amazon.sampleapp.lvc;

import android.content.Context;
import android.os.Environment;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.widget.Toast;

import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.ConfigurationFile;
import com.amazon.aace.localSkillService.config.LocalSkillServiceConfiguration;
import com.amazon.aace.localVoiceControl.config.LocalVoiceControlConfiguration;
import com.amazon.sampleapp.core.ModuleFactoryInterface;
import com.amazon.sampleapp.core.SampleAppContext;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import static android.content.ContentValues.TAG;

public class LvcModuleFactory implements ModuleFactoryInterface{

    public List<Fragment> getFragments(SampleAppContext sampleAppContext) {
        List<Fragment> list = new ArrayList<>();
        return list;
    }

    public List<Integer> getLayoutResourceNums() {
        List<Integer> list = new ArrayList<>();
        return list;
    }

    public List<PlatformInterface> getModulePlatformInterfaces(SampleAppContext sampleAppContext) {
        List<PlatformInterface> platformInterfaceList = new ArrayList<>();
        return platformInterfaceList;
    }

    public List<EngineConfiguration> getConfiguration(SampleAppContext sampleAppContext) {
        List<EngineConfiguration> configList = new ArrayList<>();

        String json = sampleAppContext.getData(SampleAppContext.JSON);
        JSONObject config = null;

        if (json != null) {
            try {
                // get required configurations from LVC config provided by LVC APK
                config = new JSONObject(json);

                JSONObject aheNode = config.getJSONObject("AlexaHybridEngine");

                JSONObject aheECNode = aheNode.getJSONObject("ExecutionController");
                String ecSocketDir = aheECNode.getString("PlatformInterfaceSocketPath");
                String ecSocketPermission = aheECNode.getString("PlatformInterfaceSocketPermissions");
                String ecMRSocketDir = aheECNode.getString("MessageRouterSocketPath");

                JSONObject ahePINode = aheNode.getJSONObject("PlatformInterfaces");
                String psSocketDir = ahePINode.getString("UnixDomainSocketPath");
                String psSocketPermission = ahePINode.getString("UnixDomainSocketPermissions");

                JSONObject aheAMNode = aheNode.getJSONObject("ArtifactManager");
                String amIngestionEP = aheAMNode
                        .getJSONObject("IngestionEP")
                        .getString("UnixDomainSocketPath");

                // The same as provided by ILVCClient.configure() in LVCInteractionService
                JSONObject aaceNode = config.getJSONObject("AACE");
                String lssSocketPath = aaceNode
                        .getJSONObject("LocalSkillService")
                        .getString("UnixDomainSocketPath");

                String localAddress = "127.0.0.1";

                configList.add(LocalVoiceControlConfiguration.createIPCConfig(
                        ecSocketDir,
                        LocalVoiceControlConfiguration.stringToSocketPermission(ecSocketPermission),
                        psSocketDir,
                        LocalVoiceControlConfiguration.stringToSocketPermission(psSocketPermission),
                        localAddress,
                        ecMRSocketDir));

                configList.add(LocalSkillServiceConfiguration.createLocalSkillServiceConfig(
                        lssSocketPath,
                        amIngestionEP));
            }
            catch( JSONException e ) {
                Log.e(TAG, "Error parsing configuration from LVC", e);
            }
        } else {
            String appDataDir = sampleAppContext.getData(SampleAppContext.APPDATA_DIR);
            configList.add(LocalVoiceControlConfiguration.createIPCConfig(
                    appDataDir,
                    LocalVoiceControlConfiguration.SocketPermission.OWNER,
                    appDataDir,
                    LocalVoiceControlConfiguration.SocketPermission.OWNER,
                    "127.0.0.1",
                    appDataDir));
            configList.add(LocalSkillServiceConfiguration.createLocalSkillServiceConfig(
                    appDataDir + "/LSS.socket",
                    appDataDir + "/ER.socket" ));
        }

        return configList;
    }
}
