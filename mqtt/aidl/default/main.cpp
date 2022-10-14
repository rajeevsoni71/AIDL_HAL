/**
 * (C) Copyright OEM
 * All rights reserved
 * @ Author: Rajeev Soni
 * @ Create Time: 2022-10-03 17:18:52
 * @ Modified by: Your name
 * @ Modified time: 2022-10-12 18:15:37
 * @ Description:
 */

#include "mqtt-impl/Mqtt.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>

using aidl::android::hardware::mqtt::Mqtt;

int main(int argc, const char** argv) {
    LOGD("%s: -Main started\n", __func__);
    std::string ip{};
    // Enable vndbinder to allow vendor-to-venfor binder call
    android::ProcessState::initWithDriver("/dev/vndbinder");
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    ABinderProcess_startThreadPool();
    if (argc < 2) {
        LOGE("%s: IP Address is missing at command line\n", __func__);
        return EXIT_FAILURE;
    }
    ip.append(argv[1]);
    LOGD("%s: IP Address :%s\n", __func__,ip.c_str());
    // make a default mqtt service
    auto mqttclient = ndk::SharedRefBase::make<Mqtt>();
    const std::string MqttName = std::string() + Mqtt::descriptor + "/default";
    binder_status_t status =
            AServiceManager_addService(mqttclient->asBinder().get(), MqttName.c_str());
    CHECK(status == STATUS_OK);
    Mqtt::setIpAddress(ip);
    mqttclient->Init();
    // ABinderProcess_joinThreadPool();
    LOGE("%s: -main End\n", __func__);
    return EXIT_FAILURE;  // should not reach
}