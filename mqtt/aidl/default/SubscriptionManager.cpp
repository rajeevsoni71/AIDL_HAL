/**
 * (C) Copyright OEM
 * All rights reserved
 * @ Author: Rajeev Soni
 * @ Create Time: 2022-10-03 17:18:06
 * @ Modified by: Your name
 * @ Modified time: 2022-10-12 18:16:23
 * @ Description:
 */

#include "mqtt-impl/SubscriptionManager.h"

namespace aidl {
namespace android {
namespace hardware {
namespace mqtt {

using namespace android;
::android::wp<SubscriptionManager> SubscriptionManager::sSubscriptionManager = nullptr;

void SubscriptionManager::onBinderDied(void* cookie) {
    ClientDeathCookie* clientCookie = static_cast<ClientDeathCookie*>(cookie);
    ::android::sp<SubscriptionManager> manager = clientCookie->mSubscriptionManager.promote();
    if (!manager) {
        return;
    }
    ClientId clientId = clientCookie->mClientId;
    manager->onCallbackDead(clientId);
}

StatusCode SubscriptionManager::addOrUpdateSubscription(
        ClientId clientId, const std::shared_ptr<IMqttCallback>& callback, std::string topic,
        int32_t qos) {
    LOGI("%s: SubscriptionManager::addOrUpdateSubscription, callback: %p, clientId:%lu\n", __func__, callback.get(), clientId);
    MuxGuard g(mLock);

    const std::shared_ptr<HalClient>& client = getOrCreateHalClientLocked(clientId, callback, topic, qos);
    if (client.get() == nullptr) {
        LOGI("%s: SubscriptionManager::addOrUpdateSubscription: StatusCode::INTERNAL_ERROR",__func__);
        return StatusCode::INTERNAL_ERROR;
    }
    LOGI("%s: SubscriptionManager::addOrUpdateSubscription: StatusCode::OK",__func__);
    return StatusCode::OK;
}

StatusCode SubscriptionManager::removeSubscription(ClientId clientId) {
    LOGI("%s: SubscriptionManager::removeSubscription, clientId %lu",__func__ ,clientId);
    auto it = mClients.find(clientId);
    auto clientCookieItem = mClientDeathCookies.find(clientId);
    if (it == mClients.end() || clientCookieItem == mClientDeathCookies.end()) {
        LOGI("%s: did not find client with id %lu",__func__, clientId);
        return StatusCode::INTERNAL_ERROR;
    } else {
        auto client = it->second;
        auto clientCookie = clientCookieItem->second;
        auto res = ndk::ScopedAStatus::fromStatus(AIBinder_unlinkToDeath(
                client->getCallback()->asBinder().get(), mDeathRecipient.get(), clientCookie));
        if (!res.isOk()) {
            LOGE("%s: failed to unlinkToDeath client: %p err: Res is not OK", __func__, client->getCallback().get());
        }
        mClients.erase(it);
        delete clientCookie;
        mClientDeathCookies.erase(clientId);
        return StatusCode::OK;
    }
}

std::shared_ptr<HalClient> SubscriptionManager::getOrCreateHalClientLocked(
        ClientId clientId, const std::shared_ptr<IMqttCallback>& callback, std::string topic,
        int32_t qos) {
    auto it = mClients.find(clientId);

    if (it == mClients.end()) {
        uint64_t cookie = reinterpret_cast<uint64_t>(clientId);
        LOGI("%s: Creating new client and linking to death recipient, cookie:%lu ",__func__, cookie);
        ClientDeathCookie* clientCookie = new ClientDeathCookie(this, cookie);
        auto res = ndk::ScopedAStatus::fromStatus(AIBinder_linkToDeath(
                callback->asBinder().get(), mDeathRecipient.get(), clientCookie));
        if (!res.isOk()) {  // Client is already dead?
            LOGE("%s: failed to link to death, cookie:%lu",__func__, cookie);
            delete clientCookie;
            return nullptr;
        }

        std::shared_ptr<HalClient> client = (std::shared_ptr<HalClient>)new HalClient(callback,topic, qos);
        mClients.insert({clientId, client});
        mClientDeathCookies.insert({clientId, clientCookie});
        return client;
    } else {
        return it->second;
    }
}

void SubscriptionManager::onCallbackDead(uint64_t cookie) {
    LOGI("%s: cookie:%lu",__func__, cookie);
    ClientId clientId = cookie;

    {
        MuxGuard g(mLock);
        const auto& it = mClients.find(clientId);
        if (it == mClients.end()) {
            LOGI("%s: cookie end: Nothing to do here, client wasn't subscribed to any properties:%lu",__func__, cookie);
            return;  // Nothing to do here, client wasn't subscribed to any properties.
        }
        const auto& halClient = it->second;
        UNUSEDPARAM(halClient);
        mClients.erase(it);

        auto clientCookieItem = mClientDeathCookies.find(clientId);
        if (clientCookieItem == mClientDeathCookies.end()) {
            return;
        }
        auto clientCookie = clientCookieItem->second;
        delete clientCookie;
        mClientDeathCookies.erase(clientId);
    }
}

std::map<ClientId, std::shared_ptr<HalClient>> SubscriptionManager::getClients() {
    return mClients;
}

}  // namespace mqtt
}  // namespace hardware
}  // namespace android
}  // namespace aidl