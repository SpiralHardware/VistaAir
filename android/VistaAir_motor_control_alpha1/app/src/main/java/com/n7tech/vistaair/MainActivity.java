package com.n7tech.vistaair;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Button;

import com.neovisionaries.ws.client.OpeningHandshakeException;
import com.neovisionaries.ws.client.WebSocket;
import com.neovisionaries.ws.client.WebSocketAdapter;
import com.neovisionaries.ws.client.WebSocketException;
import com.neovisionaries.ws.client.WebSocketFactory;

import java.io.IOException;
import java.net.URI;

public class MainActivity extends AppCompatActivity {

    WebSocketFactory wsFactory = new WebSocketFactory();

    WebSocket ws;

    protected class ConnectWebSocket extends AsyncTask<String, Void, Void> {

        protected Void doInBackground(String... arrgh) {
            try {
                ws = new WebSocketFactory().createSocket("ws://192.168.1.134:81", 120000);        // Register a listener to receive WebSocket events.
                // socket to talk to remote arduino
                try {
                    ws.addListener(new WebSocketAdapter() {
                        @Override
                        public void onTextMessage(WebSocket websocket, String message) throws Exception {
                            // Received a text message.
                        }
                    });
                    ws.connect();
                } catch (OpeningHandshakeException e)
                {
                    // A violation against the WebSocket protocol was detected
                    // during the opening handshake.
                    e.printStackTrace();
                }
                catch (WebSocketException e)
                {
                    // Failed to establish a WebSocket connection.
                    e.printStackTrace();
                }
            } catch (Exception e) {

            }
            return null;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });

        new ConnectWebSocket().execute();

        // add onTouch listeners to buttons ...
        View upButton = findViewById(R.id.button);
        upButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    ws.sendText("up");
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    ws.sendText("stop");
                }
                return true;
            };
        });

        View downButton = findViewById(R.id.button2);
        downButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    ws.sendText("down");
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    ws.sendText("stop");
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
