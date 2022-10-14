/**
 * (C) Copyright OEM
 * All rights reserved
 * @ Author: Rajeev Soni
 * @ Create Time: 2022-10-03 17:18:06
 * @ Modified by: Your name
 * @ Modified time: 2022-10-12 18:16:16
 * @ Description:
 */
#ifndef AIDL_ANDROID_HARDWARE_MQTT_SUBCRIPTION_MANAGER_H_
#define AIDL_ANDROID_HARDWARE_MQTT_SUBCRIPTION_MANAGER_H_

#include "common_utility.h"

#include <aidl/android/hardware/mqtt/IMqttCallback.h>
#include <utils/RefBase.h>
#include <android-base/logging.h>
#include <list>
#include <map>
#include <memory>
#include <set>

namespace aidl {
namespace android {
namespace hardware {
namespace mqtt {

using namespace android;

class HalClient : public ::android::RefBase {
  public:
    HalClient(const std::shared_ptr<IMqttCallback>& callback, std::string topic, int32_t qos)
        : mCallback(callback), mTopic(topic), mQos(qos) {}

    virtual ~HalClient() {}

  public:
    std::shared_ptr<IMqttCallback> getCallback() const { return mCallback; }
    std::string getTopic() const { return mTopic; }
    int32_t getQos() const { return mQos; }

  private:
    const std::shared_ptr<IMqttCallback> mCallback;
    std::string mTopic;
    int32_t mQos;
};

class SubscriptionManager : public ::android::RefBase {
  public:
    static ::android::sp<SubscriptionManager> getSubscriptionMgrInstance(){
      ::android::sp<SubscriptionManager> pSubscriptionManager = sSubscriptionManager.promote();
      if(pSubscriptionManager == nullptr) {
          pSubscriptionManager = new SubscriptionManager();
          sSubscriptionManager = pSubscriptionManager;
      }
      LOGD("%s: Returning pSubscriptionManager object ...",__func__);
      return pSubscriptionManager;
    }

    virtual ~SubscriptionManager() = default;

    StatusCode addOrUpdateSubscription(ClientId clientId,
                                       const std::shared_ptr<IMqttCallback>& callback,
                                       std::string topic, int32_t qos);
    StatusCode removeSubscription(ClientId);

    std::map<ClientId, std::shared_ptr<HalClient>> getClients();

  private:
    static ::android::wp<SubscriptionManager> sSubscriptionManager;
    /**
     * Constructs SubscriptionManager
     */
    SubscriptionManager() : mDeathRecipient(AIBinder_DeathRecipient_new(onBinderDied)) {}

    std::shared_ptr<HalClient> getOrCreateHalClientLocked(
            ClientId callingPid, const std::shared_ptr<IMqttCallback>& callback, std::string topic,
            int32_t qos);
    void onCallbackDead(uint64_t cookie);

    static void onBinderDied(void* cookie);

  private:
    struct ClientDeathCookie {
        ClientDeathCookie(const ::android::wp<SubscriptionManager>& subscriptionManager,
                          const ClientId& clientId)
            : mSubscriptionManager(subscriptionManager), mClientId(clientId) {}

        ::android::wp<SubscriptionManager> mSubscriptionManager;
        ClientId mClientId;
    };

  private:
    using MuxGuard = std::lock_guard<std::mutex>;

    mutable std::mutex mLock;

    std::map<ClientId, std::shared_ptr<HalClient>> mClients;
    std::map<ClientId, ClientDeathCookie*> mClientDeathCookies;
    ::ndk::ScopedAIBinder_DeathRecipient mDeathRecipient;
};

}  // namespace mqtt
}  // namespace hardware
}  // namespace android
}  // namespace aidl

#endif  // AIDL_ANDROID_HARDWARE_MQTT_SUBCRIPTION_MANAGER_H_