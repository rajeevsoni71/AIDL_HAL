/**
 * (C) Copyright OEM
 * All rights reserved
 * @ Author: Rajeev Soni
 * @ Create Time: 2022-10-03 17:18:06
 * @ Modified by: Your name
 * @ Modified time: 2022-10-12 18:16:07
 * @ Description:
 */
#ifndef AIDL_ANDROID_HARDWARE_MQTT_MQTT_H_
#define AIDL_ANDROID_HARDWARE_MQTT_MQTT_H_

#include <aidl/android/hardware/mqtt/BnMqtt.h>
#include <aidl/android/hardware/mqtt/BnMqttCallback.h>
#include <cassert>
#include <csignal>
#include <cstring>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include "common_utility.h"
#include <utils/RefBase.h>
#include "mqtt-impl/SubscriptionManager.h"

namespace aidl {
namespace android {
namespace hardware {
namespace mqtt {

static constexpr lws_mqtt_client_connect_param_t mqtt_connect_structure = {
        .client_id = "Mqtt_hal_Client", .keep_alive = 60, .clean_start = 1};


class Mqtt : public BnMqtt {
  public:
    virtual ~Mqtt() = default;
    void Init();
    ndk::ScopedAStatus subscribeCallbackTopic(
            const std::shared_ptr<::aidl::android::hardware::mqtt::IMqttCallback>& in_callback,
            const std::string& in_topic, int32_t in_qos, int32_t* _aidl_return) override;
    ndk::ScopedAStatus unsubscribeCallbackTopic(
            const std::shared_ptr<::aidl::android::hardware::mqtt::IMqttCallback>& in_callback,
            const std::string& in_topic, int32_t in_qos, int32_t* _aidl_return) override;
    ndk::ScopedAStatus publishData(const std::string& in_topic, const std::string& in_data,
                                   int32_t* _aidl_return) override;

    static bool sInterrupt;
    static sharedData sSharedData;
    struct lws_context* mServiceContext = nullptr;
    struct lws_context_creation_info mContextCreationInfo {};

    static void setIpAddress(std::string& ipaddress);
  private:
    static std::string sIpAddress;
    static ::android::sp<SubscriptionManager> spSubscriptionMgr;

    ClientId getClientId(const std::shared_ptr<::aidl::android::hardware::mqtt::IMqttCallback>& callback);
    static void sigint_handler(int sig);
    static int system_notify_cb(lws_state_manager_t* mgr, lws_state_notify_link_t* link,
                                int current, int target);
    static int callback_mqtt(struct lws* wsi, enum lws_callback_reasons reason, void* user,
                             void* in, size_t len);

    static int connect_client(struct lws_context* context);
};

}  // namespace mqtt
}  // namespace hardware
}  // namespace android
}  // namespace aidl

#endif  // AIDL_ANDROID_HARDWARE_MQTT_MQTT_H_