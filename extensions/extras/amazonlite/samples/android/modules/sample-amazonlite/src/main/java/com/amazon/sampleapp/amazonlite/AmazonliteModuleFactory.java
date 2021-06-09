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

package com.amazon.sampleapp.amazonlite;

import android.support.v4.app.Fragment;

import com.amazon.aace.amazonlite.config.AmazonLiteConfiguration;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.PlatformInterface;
import com.amazon.sampleapp.core.ModuleFactoryInterface;
import com.amazon.sampleapp.core.SampleAppContext;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class AmazonliteModuleFactory implements ModuleFactoryInterface{

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
        AmazonLiteConfiguration.ModelConfig[] modelsInfoList = new AmazonLiteConfiguration.ModelConfig[]{
                new AmazonLiteConfiguration.ModelConfig("de-DE", "D.de-DE.alexa.bin"),
                new AmazonLiteConfiguration.ModelConfig("en-IN", "D.en-IN.alexa.bin"),
                new AmazonLiteConfiguration.ModelConfig("en-US", "D.en-US.alexa.bin"),
                new AmazonLiteConfiguration.ModelConfig("es-ES", "D.es-ES.alexa.bin"),
                new AmazonLiteConfiguration.ModelConfig("fr-FR", "D.fr-FR.alexa.bin"),
                new AmazonLiteConfiguration.ModelConfig("it-IT", "D.it-IT.alexa.bin"),
                new AmazonLiteConfiguration.ModelConfig("ja-JP", "D.ja-JP.alexa.bin"),
                new AmazonLiteConfiguration.ModelConfig("en-GB", "U_250k.en-GB.alexa.bin"),
                new AmazonLiteConfiguration.ModelConfig("en-AU", "D.en-US.alexa.bin"), // Mapped to en_US
                new AmazonLiteConfiguration.ModelConfig("en-CA", "D.en-US.alexa.bin"), // Mapped to en_US
                new AmazonLiteConfiguration.ModelConfig("fr-CA", "D.fr-FR.alexa.bin"), // Mapped to fr_FR
                new AmazonLiteConfiguration.ModelConfig("es-MX", "D.es-ES.alexa.bin")  // Mapped to es_ES
        };

        // Uncomment the below line to enable the runtime switching of the wakeword model.
        // Also, ensure that the wakeword models are copied from the package to the assets/models folder.
        // configList.add(AmazonLiteConfiguration.createAmazonLiteConfig(sampleAppContext.getData(SampleAppContext.MODEL_DIR), modelsInfoList));

        return configList;
    }
}
