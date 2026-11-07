/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// The stock audio@2.0-service, except that the process talks to /dev/binder instead
// of /dev/vndbinder: the MTK audio blob asks the main servicemanager for NvRAMAgent
// (nvram_agent_binder), as it did when it ran inside audioserver.

#define LOG_TAG "audiohalservice"

#include <android/hardware/audio/2.0/IDevicesFactory.h>
#include <android/hardware/audio/effect/2.0/IEffectsFactory.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>

using namespace android::hardware;
using android::OK;

int main(int /* argc */, char* /* argv */ []) {
    ::android::ProcessState::initWithDriver("/dev/binder");
    ::android::ProcessState::self()->startThreadPool();

    configureRpcThreadpool(16, true /*callerWillJoin*/);

    LOG_ALWAYS_FATAL_IF(
        registerPassthroughServiceImplementation<audio::V2_0::IDevicesFactory>() != OK,
        "Could not register audio core API 2.0");
    LOG_ALWAYS_FATAL_IF(
        registerPassthroughServiceImplementation<audio::effect::V2_0::IEffectsFactory>() != OK,
        "Could not register audio effect API 2.0");

    joinRpcThreadpool();
}
