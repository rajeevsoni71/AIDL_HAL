/**
 * (C) Copyright OEM
 * All rights reserved
 * @ Author: Rajeev Soni
 * @ Create Time: 2022-10-03 18:35:15
 * @ Modified by: Your name
 * @ Modified time: 2022-10-12 18:15:53
 * @ Description:
 */
#ifndef AIDL_ANDROID_HARDWARE_MQTT_COMMON_UTILITY_H_
#define AIDL_ANDROID_HARDWARE_MQTT_COMMON_UTILITY_H_

#include <android/log.h>
#include <libwebsockets.h>
#include <string>

namespace aidl {
namespace android {
namespace hardware {
namespace mqtt {

#define TAG "mqtt_hal"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,    TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,     TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,     TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,    TAG, __VA_ARGS__)

#define UNUSEDPARAM(x) (void)(x)
#define HOSTPORTNUMBER 1883
#define PAYLOADCOMPLETED 1

using ClientId = uint64_t;

enum class StatusCode: uint32_t {
    OK,
    ACCESS_DENIED,
    INTERNAL_ERROR,
    ILLEGAL_STATE,
};

struct PerSessionData {
    lws_mqtt_publish_param_t	pub_param;
	int		                    state;
	size_t		                pos;
	int		                    retries;
};

struct sharedData {
    struct lws_context* context;
    struct lws* wsi;
    lws_sorted_usec_list_t sul;
};


const lws_retry_bo_t RetryBackOff = {
    .secs_since_valid_ping = 20,
    .secs_since_valid_hangup = 25
};

}  // namespace mqtt
}  // namespace hardware
}  // namespace android
}  // namespace aidl

#endif // AIDL_ANDROID_HARDWARE_MQTT_COMMON_UTILITY_H