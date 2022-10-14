# AIDL_FOR_HAL_MQTT

1. place the whole folder under

    $hardware/interfaces/

2. After folder is copied into above directory then run below command at your aosp code base

    $mmm hardware/interfaces/mqtt

3. you will get some error and that gerror say to execute below command first before running above command

    $m android.hardware.mqtt-update-api

4. Step 2

5. add the modules in product packages of your target build.

    #Mqtt Hal
    PRODUCT_PACKAGES += \
        android.hardware.mqtt \
        android.hardware.mqtt-service \
        MqttTestAppSubscribe \
        MqttTestAppPublish

6. copy the sepoilcy into your sepolicy folder direclty

7. do full make after deleted intermediates from out folder just to check everything is OK

    $make -j8

# Important links

https://www.codeinsideout.com/blog/android/hal/aidl/#build-and-run


# Important note:

aidl_api directory inside mqtt directory should be created by developer itself.

 $mkdir -p hardware/interfaces/mqtt/aidl/aidl_api/android.hardware.mqtt/1
