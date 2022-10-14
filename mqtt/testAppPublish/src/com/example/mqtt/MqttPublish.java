package com.example.mqtt;

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

public class MqttPublish extends Activity {
    private static final String TAG = "MqttTestAppPublish";
    private static final String IMQTT_AIDL_INTERFACE = "android.hardware.mqtt.IMqtt/default";
    private static IMqtt mqttAJ; // AIDL Java

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button btn1 = (Button)findViewById(R.id.button1);

        btn1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //Topic
                EditText editText = (EditText)findViewById(R.id.editText1);
                String txt = editText.getText().toString();
                Log.d(TAG, "App: Publish Topic" + txt);
                //Data
                EditText editText2 = (EditText)findViewById(R.id.editText2);
                String txt2 = editText2.getText().toString();
                Log.d(TAG, "App: Publish data= " + txt2);

                if(mqttAJ != null) {
                    try {
                        mqttAJ.publishData(txt, txt2);
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
