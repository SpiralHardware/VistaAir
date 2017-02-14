package com.n7tech.vistaairwificonfig;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.DhcpInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.EditorInfo;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;


/**
 * A login screen that offers login via email/password.
 */
public class WifiConfigActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener {

    // another call back - but this time from withing the UI thread!? wtf is this needed for...
    // when we select an item in the list, we have to tell the list about the item that was selected?
    // this is retarded
    public void onItemSelected(AdapterView<?> parent, View view,
                               int pos, long id) {
        ssidSelect = (Spinner) findViewById(R.id.ssid);
        ssidSelect.setSelection(pos);
    }

    public void onNothingSelected(AdapterView<?> parent) {
        // Another interface callback
    }

    // UI references.
    private Spinner ssidSelect;
    private EditText mPasswordView;
    private View mLoginFormView;
    public CharSequence esp32Status = "not connected";

    private DhcpInfo esp32AP;

    protected class ConnectToESP extends AsyncTask<String, Void, Void> {
        private String[] ssidList;

        protected Void doInBackground(String... arrgh) {
            try {
                // connect to ESP wifi Access Point
                String esp32ssid = "ESP32";
                WifiConfiguration wifiConfig = new WifiConfiguration();
                wifiConfig.SSID = "\"" + esp32ssid + "\"";
                wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
                wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
                wifiConfig.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
                wifiConfig.allowedProtocols.set(WifiConfiguration.Protocol.WPA);
                wifiConfig.allowedAuthAlgorithms.clear();
                wifiConfig.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
                wifiConfig.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
                wifiConfig.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
                wifiConfig.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP104);
                wifiConfig.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
                wifiConfig.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);

                WifiManager wifiManager = (WifiManager) getSystemService(WIFI_SERVICE);

                /* most frustratingly - android changed in v6 (see here https://code.google.com/p/android/issues/detail?id=192622)
                   to disallow changes to networks that have been added by something other than the current app.
                   So if ESP32 network already exists, we can't add him - stupid, and apparently inconsistent; see comment 25 on above link.
                   Anyway - upshot is, to work on android 6 and earlier versions, we have to scan the networks for ESP32 and
                   connect to him if he exists, or if not then we can create him.

                   For later - when managing multiple VistaAirs, we will assume each one has a uniquely preconfigured
                   SSID name, but that always starts with given Prefix - eg VistaAir - then we can just iterate through them
                   to set up wifi connections.
                 */
                List<WifiConfiguration> listConfig = wifiManager.getConfiguredNetworks();
                WifiConfiguration tmpConfig;

                boolean alreadyConfigured = false;
                for (int i = 0; i < listConfig.size(); i++) {
                    tmpConfig = listConfig.get(i);
                    if (tmpConfig.SSID != null && tmpConfig.SSID.equalsIgnoreCase(wifiConfig.SSID)) {
                        alreadyConfigured = true;

                        // it just gets worse, apparently since lollipop (api level 22) enableNetwork no longer
                        // connects if that network has not internet access ARRRBGGGGHHHHH
                        // api doc has some bullshit about going low level to tcp sockets but
                        // stackoverflow just suggests disabling other networks : http://stackoverflow.com/questions/26986023/wificonfiguration-enable-network-in-lollipop
                        //if (wifiManager.enableNetwork(tmpConfig.networkId, true)) {
                         //   esp32AP = wifiManager.getDhcpInfo();
                         //   esp32Status = "Connected to ESP32 at : "+convert2Bytes(esp32AP.serverAddress);

                        //}
                        //commented out the if statement, because it's irrelevant, getting the dhcp info is mostly likely going to be
                        //wrong until we've disabled all the other networks apart from the one we want.
                        wifiManager.disconnect();
                        wifiManager.enableNetwork(tmpConfig.networkId, true);
                        wifiManager.reconnect();
                        wifiManager.getConnectionInfo().getIpAddress();
                    } else {
                        wifiManager.disableNetwork(tmpConfig.networkId);
                    }
                }

                // having checked list of known wifi configs and not found esp32, then we can try and create him.
                if (!alreadyConfigured) {
                    // try to create and manually connect
                    wifiManager.addNetwork(wifiConfig);

                    // it just gets worse, apparently since lollipop (api level 22) enableNetwork no longer
                    // connects if that network has not internet access ARRRBGGGGHHHHH
                    // that, inspite of the existing setting disableOthers, set to true....
//                    if (wifiManager.enableNetwork(wifiConfig.networkId, true)) {
//                        esp32AP = wifiManager.getDhcpInfo();
//                        esp32Status = "Connected to ESP32 at : "+convert2Bytes(esp32AP.serverAddress);
//                    }
                    wifiManager.disconnect();
                    wifiManager.enableNetwork(wifiConfig.networkId, true);
                    wifiManager.reconnect();
                    wifiManager.getConnectionInfo().getIpAddress();
                }
                esp32AP = wifiManager.getDhcpInfo();
                WifiInfo test = wifiManager.getConnectionInfo();
                String clientIp = InetAddress.getByAddress(convert2Bytes(test.getIpAddress())).getHostAddress();
                esp32Status = "Connected to ESP32 at : "+InetAddress.getByAddress(convert2Bytes(esp32AP.serverAddress)).getHostAddress();
                CharSequence debugStr = esp32Status;
            } catch (Exception e) {
                int dammit = 0;
            }
            return null;
        }

        protected void onPostExecute(){
            EditText connectionStatus = (EditText) findViewById(R.id.ESPConnectionStatus);
            connectionStatus.setText(esp32Status);
        }

    }

    // can't do network stuff on ui thread, so start task to run in background
    // and upload wifi config to ESP32
    protected class SendWifiConfig extends AsyncTask<String, Void, Void> {

        private String _ssid;
        private String _password;

        public SendWifiConfig(String ssid, String pass){
            super();
            _ssid = ssid;
            _password = pass;
        }

        protected Void doInBackground(String... arrgh) {
            try {
                HttpURLConnection urlConnection = null;
                byte[] ipAddress = convert2Bytes(esp32AP.serverAddress);
                try {
                    String apIpAddr = InetAddress.getByAddress(convert2Bytes(esp32AP.serverAddress)).getHostAddress();
                    //URL url = new URL("http://"+apIpAddr+"/"+_ssid+"/"+_password+"/blah");

                    // at one point using Dhcp to get ip address of the ESP32 AP seemed to work -
                    // but then it didn't so back to hardcoding to 192.168.4.1
                    URL url = new URL("http://192.168.4.1/"+_ssid+"/"+_password+"/blah");

                    urlConnection = (HttpURLConnection) url.openConnection();
                    InputStream in = urlConnection.getInputStream();
                    InputStreamReader isw = new InputStreamReader(in);
                    int data = isw.read();
                    while (data != -1) {
                        char current = (char) data;
                        data = isw.read();
                        System.out.print(current);
                    }

                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    if(urlConnection != null) {
                        urlConnection.disconnect();
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }
    }

    ArrayList<String> ssidList = new ArrayList<String>();

    class WifiReceiver extends BroadcastReceiver{
        private ArrayAdapter aa;

        public WifiReceiver(ArrayAdapter adaptorArg){
            super();
            aa = adaptorArg;
        }
        @Override
        public void onReceive(Context c, Intent intent)
        {
            // empty the list and refill it
            ssidList.clear();
            aa.notifyDataSetChanged();

            //     #########
            // !!! NOTA BENE !!!
            //     #########
            // For wifi scan to work - phone MUST have Location setting ON =- otherwise scan results are empty set
            // argh !!! talk about invisible failure. Even with permissions enabled etc... must also have phone with Location switched on.
           WifiManager wm =  (WifiManager) getSystemService(WIFI_SERVICE);
            List<ScanResult> scannedAPs = wm.getScanResults();
            Iterator<ScanResult> eachAP = scannedAPs.iterator();
            while(eachAP.hasNext()){
                ScanResult ap = eachAP.next();
                ssidList.add(ap.SSID);

                // can't just have an updatable drop down without reciprocating call-backs - god I hate modern code
                // this is the call back that's called after the call back that was triggered from startScan.
                // When scan results are in, and the wifi receiver processes the results and adds to the local AP
                // list in the SSID drop down, for each item we add, we have to add a call back to the list to
                // update itself in the UI display
                aa.notifyDataSetChanged();
            }

            // comment below is for old code
            // so apparently we get get multiple call backs from wifi.startScan
            // which means we're updating the drop down many times, which is apparently a no-no
            // outside of the UI thread, see here : http://stackoverflow.com/questions/3132021/android-listview-illegalstateexception-the-content-of-the-adapter-has-changed
            // of course the answer I like is the simplest one
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ArrayAdapter ssidSpinnerAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, ssidList);

        setContentView(R.layout.activity_wifi_config);

        // set up the SSID drop down select - empty to start with, but initiate wifi AP scan
        // and populate when that call back happens.
        ssidSelect = (Spinner) findViewById(R.id.ssid);

        // this is a perfect example of why I had to get out of software, since Android SDK 22, the application has to
        // both declare that it needs location access (in manifest.xml permissions), AND it has to explicitly ask the user for location access before
        // it is allowed to SCAN local wireless networks. This is just completely stupid, and yet, nothing will be done....
        // see here : https://code.google.com/p/android/issues/detail?id=185370
        // and this one with a defence which is quite good, but I would still stone the fucker https://code.google.com/p/android/issues/detail?id=231911&thanks=231911&ts=1484077807
        // WTF has "location" got to do with looking and connecting to wifi???
        // this is security concerns over idiotic trifles wagging the dog ... entropic heat death happens to all systems it seems...
        requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION, Manifest.permission.ACCESS_FINE_LOCATION}, 0x1234B);

        ssidSpinnerAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        ssidSelect.setAdapter(ssidSpinnerAdapter);
        ssidSelect.setOnItemSelectedListener(this);

        //     #########
        // !!! NOTA BENE !!!
        //     #########
        // For wifi scan to work - phone MUST have location ON =- otherwise scan results are empty set
        // argh !!!  even with permissions enabled etc... must also have phone with Location switched on.
        WifiManager wifiManager = (WifiManager) getSystemService(WIFI_SERVICE);

        // callback for results of wifi scan
        IntentFilter mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        registerReceiver(new WifiReceiver(ssidSpinnerAdapter), mIntentFilter);
        wifiManager.startScan();


        mPasswordView = (EditText) findViewById(R.id.password);
        mPasswordView.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int id, KeyEvent keyEvent) {
                if (id == R.id.login || id == EditorInfo.IME_NULL) {
                    uploadConfig();
                    return true;
                }
                return false;
            }
        });
        Button uploadButton = (Button) findViewById(R.id.upload_button);
        uploadButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                uploadConfig();
            }
        });

        mLoginFormView = findViewById(R.id.login_form);

        // again, any internet stuff has to happen off the ui thread
        new ConnectToESP().execute();

        EditText connectionStatus = (EditText) findViewById(R.id.ESPConnectionStatus);
        connectionStatus.setText(esp32Status);

    }

    private static byte[] convert2Bytes(int hostAddress) {
        byte[] addressBytes = {(byte) (0xff & hostAddress),
                (byte) (0xff & (hostAddress >> 8)),
                (byte) (0xff & (hostAddress >> 16)),
                (byte) (0xff & (hostAddress >> 24))};
        return addressBytes;
    }

    /**
     * upload wifi AP login details to the ESP
     */
    private void uploadConfig() {

        EditText connectionStatus = (EditText) findViewById(R.id.ESPConnectionStatus);
        connectionStatus.setText(esp32Status);
        // Reset errors.
        mPasswordView.setError(null);

        // Store values at the time of the login attempt.
        String password = mPasswordView.getText().toString();

        if (esp32AP != null) {

            // omce again can't do network stuff on UI thread
            new SendWifiConfig(ssidSelect.getSelectedItem().toString(), password).execute();
        }
    }

}

