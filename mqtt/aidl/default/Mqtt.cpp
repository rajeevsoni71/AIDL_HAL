/**
 * (C) Copyright OEM
 * All rights reserved
 * @ Author: Rajeev Soni
 * @ Create Time: 2022-10-03 17:18:15
 * @ Modified by: Your name
 * @ Modified time: 2022-10-12 18:16:02
 * @ Description: Mqtt hal implementation
 */

#include "mqtt-impl/Mqtt.h"
#include <android-base/logging.h>

namespace aidl {
namespace android {
namespace hardware {
namespace mqtt {

sharedData Mqtt::sSharedData;
bool Mqtt::sInterrupt = false;
std::string Mqtt::sIpAddress= {};
::android::sp<SubscriptionManager> Mqtt::spSubscriptionMgr = nullptr;

ClientId Mqtt::getClientId(const std::shared_ptr<::aidl::android::hardware::mqtt::IMqttCallback>& callback) {
    LOGI("%s :", __func__);
    if (callback->isRemote()) {
        BnMqttCallback* hwCallback = static_cast<BnMqttCallback*>(callback.get());
        return static_cast<ClientId>(reinterpret_cast<intptr_t>(hwCallback->asBinder().get()));
    } else {
        return static_cast<ClientId>(reinterpret_cast<intptr_t>(callback.get()));
    }
}

ndk::ScopedAStatus Mqtt::subscribeCallbackTopic(
        const std::shared_ptr<::aidl::android::hardware::mqtt::IMqttCallback>& in_callback,
        const std::string& in_topic, int32_t in_qos, int32_t* _aidl_return) {
    UNUSEDPARAM(_aidl_return);
    LOGD("%s: -started\n", __func__);
    lws_mqtt_topic_elem_t topic = {.name = in_topic.c_str(),
                                   .qos = static_cast<lws_mqtt_qos_levels_t>(in_qos)};
    lws_mqtt_subscribe_param_t mqtt_subscribe_structure = {
            .num_topics = 1,
            .topic = &topic,
    };

    if (lws_mqtt_client_send_subcribe(Mqtt::sSharedData.wsi, &mqtt_subscribe_structure)) {
        LOGD("%s: subscribe failed\n", __func__);
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    StatusCode retStatus = Mqtt::spSubscriptionMgr->addOrUpdateSubscription(getClientId(in_callback),
                                       in_callback,
                                       in_topic, in_qos);
    if (retStatus != StatusCode::OK){
        LOGE("%s, failed to add callback into subscription list...", __func__);
    }
    LOGD("%s: -End\n", __func__);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Mqtt::unsubscribeCallbackTopic(
        const std::shared_ptr<::aidl::android::hardware::mqtt::IMqttCallback>& in_callback,
        const std::string& in_topic, int32_t in_qos, int32_t* _aidl_return) {
    UNUSEDPARAM(_aidl_return);
    LOGD("%s: -started\n", __func__);
    lws_mqtt_topic_elem_t topic = {.name = in_topic.c_str(),
                                   .qos = static_cast<lws_mqtt_qos_levels_t>(in_qos)};
    lws_mqtt_subscribe_param_t mqtt_subscribe_structure = {
            .num_topics = 1,
            .topic = &topic,
    };
    if (lws_mqtt_client_send_unsubcribe(Mqtt::sSharedData.wsi, &mqtt_subscribe_structure)) {
        LOGE("%s: Unsubscribe failed\n", __func__);
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    StatusCode retStatus = Mqtt::spSubscriptionMgr->removeSubscription(getClientId(in_callback));
    if (retStatus != StatusCode::OK){
        LOGE("%s, failed to remove callback from removeSubscription list...", __func__);
    }
    LOGD("%s: -End\n", __func__);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Mqtt::publishData(const std::string& in_topic, const std::string& in_data,
                                     int32_t* _aidl_return) {
    UNUSEDPARAM(_aidl_return);
    LOGD("%s: -started\n", __func__);
    char PublishTopic[128] = {0};
    strncpy(PublishTopic, (char*)in_topic.c_str(), in_topic.length());
    lws_mqtt_publish_param_t publishParam;
    publishParam.topic = PublishTopic;
    publishParam.topic_len = (uint16_t)strlen(publishParam.topic);
    publishParam.qos = QOS0;
    publishParam.payload_len = in_data.length();

    if (lws_mqtt_client_send_publish(Mqtt::sSharedData.wsi, &publishParam, (void*)in_data.c_str(),
                                     publishParam.payload_len, PAYLOADCOMPLETED)) {
        LOGE("%s: Publish data failed\n", __func__);
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    LOGD("%s: -End\n", __func__);
    return ndk::ScopedAStatus::ok();
}

int Mqtt::callback_mqtt(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in,
                        size_t len) {
    UNUSEDPARAM(len);
    UNUSEDPARAM(user);
    lws_mqtt_publish_param_t* pub;
    switch (reason) {
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
            LOGE("%s: LWS_CALLBACK_CLIENT_CONNECTION_ERROR: %s\n", __func__,
                 in ? (char*)in : "(null)");
            Mqtt::sInterrupt = true;
            break;
        }
        case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION: {
            LOGE("%s: LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION: %s\n", __func__,
                 in ? (char*)in : "(null)");
            sleep(3);
            break;
        }
        case LWS_CALLBACK_MQTT_NEW_CLIENT_INSTANTIATED: {
            LOGD("%s: LWS_CALLBACK_MQTT_NEW_CLIENT_INSTANTIATED :\n", __func__);
            Mqtt::sSharedData.wsi = wsi;
            lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_MQTT_IDLE: {
            LOGD("%s: LWS_CALLBACK_MQTT_IDLE :\n", __func__);
            break;
        }
        case LWS_CALLBACK_MQTT_CLIENT_ESTABLISHED: {
            LOGD("%s: LWS_CALLBACK_MQTT_CLIENT_ESTABLISHED :\n", __func__);
            Mqtt::sSharedData.wsi = wsi;
            lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_MQTT_SUBSCRIBED: {
            LOGD("%s: LWS_CALLBACK_MQTT_SUBSCRIBED :\n", __func__);
            Mqtt::sSharedData.wsi = wsi;
            lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_MQTT_CLIENT_WRITEABLE: {
            LOGD("%s: LWS_CALLBACK_MQTT_CLIENT_WRITEABLE :\n", __func__);
            Mqtt::sSharedData.wsi = wsi;
            break;
        }
        case LWS_CALLBACK_MQTT_CLIENT_RX: {
            LOGD("%s: LWS_CALLBACK_MQTT_CLIENT_RX\n", __func__);
            pub = (lws_mqtt_publish_param_t*)in;
            assert(pub);
            char payload_message[128] = {0};
            strncpy(payload_message, (char*)pub->payload, pub->payload_len);
            LOGI("%s: Topic:%s, topic_len:%u\n", __func__, pub->topic, pub->topic_len);
            LOGI("%s: payload:%s, payload_len:%u, payload_pos:%u\n", __func__, payload_message,
                 pub->payload_len, pub->payload_pos);
            auto clients = Mqtt::spSubscriptionMgr->getClients();
            for (const auto& it : clients) {
                it.second->getCallback()->onReceiveDataFromBroker(pub->topic,payload_message);
            }
            return 0;
        }
        case LWS_CALLBACK_MQTT_UNSUBSCRIBED: {
            LOGE("%s: LWS_CALLBACK_MQTT_UNSUBSCRIBED: %s\n", __func__, in ? (char*)in : "(null)");
            break;
        }
        case LWS_CALLBACK_MQTT_DROP_PROTOCOL: {
            LOGE("%s: LWS_CALLBACK_MQTT_DROP_PROTOCOL: %s\n", __func__, in ? (char*)in : "(null)");
            break;
        }
        case LWS_CALLBACK_MQTT_CLIENT_CLOSED: {
            LOGE("%s: LWS_CALLBACK_MQTT_CLIENT_CLOSED: %s\n", __func__, in ? (char*)in : "(null)");
            Mqtt::sInterrupt = true;
            break;
        }
        case LWS_CALLBACK_MQTT_ACK: {
            LOGD("%s: LWS_CALLBACK_MQTT_ACK :\n", __func__);
            Mqtt::sSharedData.wsi = wsi;
            lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_MQTT_RESEND: {
            LOGE("%s: LWS_CALLBACK_MQTT_RESEND: %s\n", __func__, in ? (char*)in : "(null)");
            break;
        }
        case LWS_CALLBACK_TIMER: {
            LOGD("%s: LWS_CALLBACK_TIMER :\n", __func__);
            break;
        }
        case LWS_CALLBACK_EVENT_WAIT_CANCELLED: {
            LOGD("%s: LWS_CALLBACK_EVENT_WAIT_CANCELLED :\n", __func__);
            break;
        }
        case LWS_CALLBACK_CHILD_CLOSING: {
            LOGD("%s: LWS_CALLBACK_CHILD_CLOSING :\n", __func__);
            break;
        }
        default: {
            LOGE("%s: Default case: %s\n", __func__, in ? (char*)in : "(null)");
            break;
        }
    }
    return 0;
}

int Mqtt::system_notify_cb(lws_state_manager_t* mgr, lws_state_notify_link_t* link, int current,
                           int target) {
    UNUSEDPARAM(link);
    struct lws_context* pContext = static_cast<lws_context*>(mgr->parent);

    if (current != LWS_SYSTATE_OPERATIONAL || target != LWS_SYSTATE_OPERATIONAL) return 0;

    /*
     * We delay trying to do the client connection until the protocols have
     * been initialized for each vhost... this happens after we have network
     * and time so we can judge tls cert validity.
     */
    if (Mqtt::connect_client(pContext)) Mqtt::sInterrupt = true;
    return 0;
}

void Mqtt::sigint_handler(int sig) {
    LOGD("%s, sig:%d", __func__, sig);
    Mqtt::sInterrupt = true;
}

void Mqtt::setIpAddress(std::string& ipaddress){
    LOGD("%s, setting ip:%s", __func__, ipaddress.c_str());
    Mqtt::sIpAddress = ipaddress;
}

/**
 * connect to mqtt broker
 *
 * @param context
 * @param mqtt_connect_structure
 * @param hostname
 * @param portnumber
 * @return int
 */
int Mqtt::connect_client(struct lws_context* context) {
    LOGD("%s: started IP:%s\n", __func__, Mqtt::sIpAddress.c_str());
    struct lws_client_connect_info clientConnectInfo;

    memset(&clientConnectInfo, 0, sizeof clientConnectInfo);

    clientConnectInfo.mqtt_cp = &mqtt_connect_structure;
    clientConnectInfo.address = Mqtt::sIpAddress.c_str();
    clientConnectInfo.host = clientConnectInfo.address;
    clientConnectInfo.protocol = "mqtt";
    clientConnectInfo.context = context;
    clientConnectInfo.method = "MQTT";
    clientConnectInfo.alpn = "mqtt";
    clientConnectInfo.port = HOSTPORTNUMBER;

    if (!lws_client_connect_via_info(&clientConnectInfo)) {
        LOGE("%s: HAL Connect Failed\n", __func__);
        return 1;
    }
    LOGD("%s: End\n", __func__);
    return 0;
}

void Mqtt::Init() {
    int ret = 0;
    LOGD("%s init started\n", __func__);
    spSubscriptionMgr = SubscriptionManager::getSubscriptionMgrInstance();
    memset(&mContextCreationInfo, 0, sizeof mContextCreationInfo);

    // Capture the signal so that we can destory the context
    std::signal(SIGINT, Mqtt::sigint_handler);

    // mapping mqtt callback
    lws_state_notify_link_t notifier = {{NULL, NULL, NULL}, Mqtt::system_notify_cb, "app"};
    lws_state_notify_link_t* mSystemStateNotifier[] = {&notifier, NULL};

    struct lws_protocols protocols[] = {{.name = "mqtt",
                                         .callback = Mqtt::callback_mqtt,
                                         .per_session_data_size = sizeof(struct PerSessionData)},
                                        {NULL, NULL, 0, 0, 0, NULL, 0}};
    // defining the context data
    mContextCreationInfo.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
    mContextCreationInfo.protocols = protocols;
    mContextCreationInfo.register_notifier_list = mSystemStateNotifier;
    mContextCreationInfo.fd_limit_per_thread = 1 + 1 + 1;
    mContextCreationInfo.retry_and_idle_policy = &RetryBackOff;

    // Creating context
    mServiceContext = lws_create_context(&mContextCreationInfo);
    if (mServiceContext == nullptr) {
        LOGE("%s libwebsocket init failed\n", __func__);
    } else {
        // Event loop
        while (ret >= 0 && !Mqtt::sInterrupt)
            ret = lws_service(mServiceContext, 0);
    }

    lws_context_destroy(mServiceContext);
    LOGD("%s: exited cleanly\n", __func__);
}

}  // namespace mqtt
}  // namespace hardware
}  // namespace android
}  // namespace aidl
