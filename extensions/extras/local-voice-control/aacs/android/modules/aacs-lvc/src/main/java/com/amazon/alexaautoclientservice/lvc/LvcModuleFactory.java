/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.lvc;

import android.util.Log;

import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.localNavigation.config.LocalNavigationConfiguration;
import com.amazon.aace.localSkillService.config.LocalSkillServiceConfiguration;
import com.amazon.aace.localVoiceControl.config.LocalVoiceControlConfiguration;
import com.amazon.alexaautoclientservice.aacs_extra.AACSModuleFactoryInterface;
import com.amazon.alexaautoclientservice.aacs_extra.AACSContext;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

public class LvcModuleFactory implements AACSModuleFactoryInterface {
    private static final String TAG = LvcModuleFactory.class.getSimpleName();

    private static final String LOCAL_NAVIGATION = "localNavigation";
    private static final String LOCAL_SEARCH_PROVIDER = "LocalSearchProvider";

    public List<PlatformInterface> getModulePlatformInterfaces(AACSContext aacsContext) {
        List<PlatformInterface> platformInterfaceList = new ArrayList<>();
        if (aacsContext.isPlatformInterfaceEnabled(LOCAL_SEARCH_PROVIDER)) {
            platformInterfaceList.add(new LocalSearchProviderHandler());
        }
        return platformInterfaceList;
    }

    public List<EngineConfiguration> getConfiguration(AACSContext aacsContext) {
        List<EngineConfiguration> configList = new ArrayList<>();

        if (aacsContext.isPlatformInterfaceEnabled(LOCAL_SEARCH_PROVIDER)) {
            // add configuration for disabling AASB's LocalSearchProvider handler
            aacsContext.deregisterAASBPlatformInterface(LOCAL_NAVIGATION, LOCAL_SEARCH_PROVIDER);
        }

        String configString = aacsContext.getData(AACSContext.JSON);
        JSONObject lvcApkConfig = null;

        if (configString != null) {
            try {
                // get required configurations from LVC config provided by LVC APK
                lvcApkConfig = new JSONObject(configString);

                JSONObject aheNode = lvcApkConfig.getJSONObject("AlexaHybridEngine");

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
                JSONObject aaceNode = lvcApkConfig.getJSONObject("AACE");
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

                JSONObject localSearchNode = aaceNode.getJSONObject("LocalSearch");
                String navPOISocketDir = localSearchNode.getString("NavigationPOISocketDir");
                String navPOISocketName = localSearchNode.getString("NavigationPOISocketName");
                String poiEERSocketDir = localSearchNode.getString("POIEERSocketDir");
                String poiEERSocketName = localSearchNode.getString("POIEERSocketName");
                configList.add(LocalNavigationConfiguration.createLocalSearchConfig(
                        navPOISocketDir + "/" + navPOISocketName,
                        poiEERSocketDir + "/" + poiEERSocketName));
            }
            catch( JSONException e ) {
                Log.e(TAG, "Error parsing configuration from LVC", e);
            }
        } else {
            String appDataDir = aacsContext.getData(AACSContext.APPDATA_DIR);
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
            configList.add(LocalNavigationConfiguration.createLocalSearchConfig(
                    appDataDir + "/navigationPOI.socket",
                    appDataDir + "/poiEER.socket"));
        }

        return configList;
    }
}
