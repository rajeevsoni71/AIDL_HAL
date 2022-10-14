/*
 * (C) Copyright OEM
 * All rights reserved
 * @ Author: Rajeev Soni
 * @ Create Time: 2022-10-03 14:48:24
 */

package android.hardware.mqtt;
import android.hardware.mqtt.IMqttCallback;

@VintfStability
interface IMqtt {
    int subscribeCallbackTopic(in IMqttCallback callback, in String topic, in int qos);
    int unsubscribeCallbackTopic(in IMqttCallback callback, in String topic, in int qos);
    int publishData(in String topic, in String data);
}
