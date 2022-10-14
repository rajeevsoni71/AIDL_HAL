package com.subcribe.mqtt;

import android.content.Context;
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.util.Log;
import android.os.ServiceManager;
import android.os.IBinder;
import android.hardware.mqtt.IMqtt;
import android.hardware.mqtt.IMqttCallback;

public class MqttSubscribe extends Activity {
    private static final String TAG = "MqttTestAppSubscribe";
    private static final String IMQTT_AIDL_INTERFACE = "android.hardware.mqtt.IMqtt/default";
    private static IMqtt mqttAJ; // AIDL Java

    class MqttListener extends IMqttCallback.Stub {
        @Override
        public void onReceiveDataFromBroker(String topic, String data){
            Log.d(TAG, "IMqtt callback received topic:" + topic);
            Log.d(TAG, "IMqtt callback received data:" + data);
            TextView tv = (TextView)findViewById(R.id.textView);
            tv.setText(topic + "," + data);
        }
        @Override
        public int getInterfaceVersion() {
            return 1;
        }

        @Override
        public String getInterfaceHash() {
            return "";
        }
    }
    private IMqttCallback mqttlistener = new MqttListener();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button btn1 = (Button)findViewById(R.id.button1);
        btn1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                EditText editText0 = (EditText)findViewById(R.id.editText1);
                String txt = editText0.getText().toString();
                Log.d(TAG, "App: Subscribe requested= " + txt);

                if(mqttAJ != null) {
                    try {
                        mqttAJ.subscribeCallbackTopic(mqttlistener, txt, 1);
                    } catch (android.os.RemoteException e) {
                        Log.e(TAG, "IMqtt-AIDL error", e);
                    }
                }
            }
        });

        Button btn2 = (Button)findViewById(R.id.button2);
        btn2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                EditText editText0 = (EditText)findViewById(R.id.editText1);
                String txt = editText0.getText().toString();
                Log.d(TAG, "App: Unsubscribe requested= " + txt);

                if(mqttAJ != null) {
                    try {
                        mqttAJ.unsubscribeCallbackTopic(mqttlistener, txt, 1);
                    } catch (android.os.RemoteException e) {
                        Log.e(TAG, "IMqtt-AIDL error", e);
                    }
                }
            }
        });

        IBinder binder = ServiceManager.getService(IMQTT_AIDL_INTERFACE);
        if (binder == null) {
            Log.e(TAG, "Getting " + IMQTT_AIDL_INTERFACE + " service daemon binder failed!");
        } else {
            mqttAJ = IMqtt.Stub.asInterface(binder);
            if (mqttAJ == null) {
                Log.e(TAG, "Getting IMqtt AIDL daemon interface failed!");
            } else {
                Log.d(TAG, "IMqtt AIDL daemon interface is binded!");
            }
        }
    }

}
