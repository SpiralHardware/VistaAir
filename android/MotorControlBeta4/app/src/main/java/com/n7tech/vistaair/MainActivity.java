package com.n7tech.vistaair;

import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.neovisionaries.ws.client.OpeningHandshakeException;
import com.neovisionaries.ws.client.WebSocket;
import com.neovisionaries.ws.client.WebSocketAdapter;
import com.neovisionaries.ws.client.WebSocketException;
import com.neovisionaries.ws.client.WebSocketFactory;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.URI;
import java.net.URL;

// would love to use Websockets as per the alpha, but currently there's no websocket lib for ESP32
// so for now - have to use good-ol-http-path-requests
// like so http://esp32-address/up/blah - http://esp32-address/stop/blah - http://esp32-address/down/blah
//
public class MainActivity extends AppCompatActivity {

    String esp32address = null;


    private static byte[] convert2Bytes(int hostAddress) {
        byte[] addressBytes = {(byte) (0xff & hostAddress),
                (byte) (0xff & (hostAddress >> 8)),
                (byte) (0xff & (hostAddress >> 16)),
                (byte) (0xff & (hostAddress >> 24))};
        return addressBytes;
    }


    // very crude - take dhcp address, chop off last number, start at 1 and
    // iterate through to 255 checking for an http server that will respond with
    // X-string (eg "ESP32" or "VISTA-AIR") - when found, save that address for
    // future comms.
    // Ideally we'd be using "network discovery" - and apparently it seems there
    // is a "bonjour" implementation for Arduino, but this crude method does have
    // the attraction of simplicity, may well be fine for finished product.
    protected class ScanForESP32 extends AsyncTask<String, Void, Void> {

        public ScanForESP32() {
            super();
        }

        protected Void doInBackground(String... arrgh) {
            HttpURLConnection urlConnection = null;
            try {
                WifiManager wm = (WifiManager) getSystemService(WIFI_SERVICE);

                String accessPointIp = InetAddress.getByAddress(convert2Bytes(wm.getConnectionInfo().getIpAddress())).getHostAddress();
                String localNetworkIpRoot = accessPointIp.substring(0, accessPointIp.lastIndexOf("."));

                // FIXME: 14/02/17
                // for now and for simplicity - just take the first one that responds and assume it's our boy; ESP32,
                // obviously this should to be updated to do some kind of connection handshake - but it's good enough
                // for proof of concept - also nothing else is likely to be using port 60
                for (int i = 0; i < 256; i++) {

                    try {
                        String checkIp = localNetworkIpRoot + "." + i;
                        Socket socket = new Socket();
                        SocketAddress socketAddress = new InetSocketAddress(checkIp, 60);

                        socket.connect(socketAddress, 60);
                        if (socket.isConnected()) {
                            // gottim - store the IP to send motor commands to
                            esp32address = checkIp;
                            socket.close();
                            return null;
                        }
                    } catch (Exception e) {
                        //e.printStackTrace();
                        int oops = 1+2;
                        // FIXME: 014 14/02/17
                        // ignore - we expect a bunch of Failed to connect exceptions, this is the
                        // priceof using this horrible crude scan method
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                if (urlConnection != null) {
                    urlConnection.disconnect();
                }
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void arg) {
            final TextView connectionStatus = (TextView) findViewById(R.id.connectedUnit);
            if(esp32address != null) {
                connectionStatus.setText(esp32address);
            } else {
                connectionStatus.setText("oops could not connect");
            }
        }
    }

    /**
     * horrific crudeness here - every time we want to send a motor command, instantiate a motor command
     * async object and create a new http connection object and all it's stuff to send a URL
     * request "stop" "up" or "down"
     * Hopefully it's responsive enough for proof of concept.
     *
     * NOTA BENE - async tasks don't seem to work here - something about a shared thread pool
     * for all AsyncTasks - eg see here http://stackoverflow.com/questions/9654148/android-asynctask-threads-limits
     * Basically they work for the first X button presses, and then stop (presumably they'll get picked up
     * some indeterminate time later, but this is of no use, button presses have to have immediate response)
     */
    protected class SendMotorCommand extends Thread{

        private String command;

        public SendMotorCommand(String c){
            super();
            command = c;
        }


        public void run() {
            try {
                HttpURLConnection urlConnection = null;
                try {
                    URL url = new URL("http://"+esp32address+":60/"+command+"/blah");
                    urlConnection = (HttpURLConnection) url.openConnection();
                    InputStream in = urlConnection.getInputStream();
                    InputStreamReader isw = new InputStreamReader(in);
                    int data = isw.read();
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
        }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        // first thing - scan for the ESP32
        new ScanForESP32().execute();

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });

        // add onTouch listeners to buttons ...
        View upButton = findViewById(R.id.button);
        upButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                Thread cmd = new SendMotorCommand("up");
                cmd.start();

            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                Thread cmd = new SendMotorCommand("stop");
                cmd.start();
            }
            return true;
        };
    });

        View downButton = findViewById(R.id.button2);
        downButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                Thread cmd = new SendMotorCommand("down");
                cmd.start();
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                Thread cmd = new SendMotorCommand("stop");
                cmd.start();
            }
            return true;
            }
        });

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
