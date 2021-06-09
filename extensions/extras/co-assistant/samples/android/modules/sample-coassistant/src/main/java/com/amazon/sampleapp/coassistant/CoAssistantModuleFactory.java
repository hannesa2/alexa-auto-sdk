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

package com.amazon.sampleapp.coassistant;

import android.support.v4.app.Fragment;
import android.util.Log;

import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.PlatformInterface;
import com.amazon.sampleapp.core.ModuleFactoryInterface;
import com.amazon.sampleapp.core.FileUtils;
import com.amazon.sampleapp.core.SampleAppContext;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

import static android.content.ContentValues.TAG;

public class CoAssistantModuleFactory implements ModuleFactoryInterface{

    @Override
    public List<Fragment> getFragments(SampleAppContext sampleAppContext) {
        List<Fragment> list = new ArrayList<>();
        return list;
    }

    @Override
    public List<Integer> getLayoutResourceNums() {
        List<Integer> list = new ArrayList<>();
        return list;
    }

    @Override
    public List<PlatformInterface> getModulePlatformInterfaces(SampleAppContext sampleAppContext) {
        List<PlatformInterface> platformInterfaceList = new ArrayList<>();
        return platformInterfaceList;
    }

    @Override
    public List<EngineConfiguration> getConfiguration(SampleAppContext sampleAppContext) {
        List<EngineConfiguration> configList = new ArrayList<>();
        return configList;
    }
}
