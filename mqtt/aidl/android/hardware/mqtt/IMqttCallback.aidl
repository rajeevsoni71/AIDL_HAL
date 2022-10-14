/*
 * (C) Copyright OEM
 * All rights reserved
 * @ Author: Rajeev Soni
 * @ Create Time: 2022-10-03 14:48:24
 */

package android.hardware.mqtt;

@VintfStability
interface IMqttCallback {
   oneway void onReceiveDataFromBroker(String topic, String data);
}