package org.harbaum.ftduinodemo;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import java.lang.ref.WeakReference;
import java.util.Set;

public class MainActivity extends AppCompatActivity {

    /*
     * Notifications from FtduinoService will be received here.
     */
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            switch (intent.getAction()) {
                case FtduinoService.ACTION_USB_PERMISSION_GRANTED: // USB PERMISSION GRANTED
                    is_connected(context, true);
                    break;
                case FtduinoService.ACTION_USB_PERMISSION_NOT_GRANTED: // USB PERMISSION NOT GRANTED
                    Toast.makeText(context, "USB Permission not granted", Toast.LENGTH_SHORT).show();
                    break;
                case FtduinoService.ACTION_NO_USB: // NO USB CONNECTED
                    break;
                case FtduinoService.ACTION_USB_DISCONNECTED: // USB DISCONNECTED
                    is_connected(context, false);
                    break;
                case FtduinoService.ACTION_USB_NOT_SUPPORTED: // USB NOT SUPPORTED
                    break;
            }
        }
    };

    private final void is_connected(Context context, boolean on) {
        if(on != connected)
            Toast.makeText(context, on?"ftDuino attached":"ftDuino detached", Toast.LENGTH_SHORT).show();

        connected = on;
        switchbutton.setEnabled(on);
        display.setEnabled(on);
        if(menu != null) {
            if(on && display != null) display.setText("");
            menu.getItem(0).setIcon(ContextCompat.getDrawable(menuContext,
                    on?R.drawable.ic_usb_connected:R.drawable.ic_usb_disconnected));
        }
    }

    private FtduinoService ftduinoService;
    private TextView display;
    private MyHandler mHandler;
    private Menu menu;
    private boolean connected = false;
    private Context menuContext = null;
    private Switch switchbutton;

    private final ServiceConnection usbConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName arg0, IBinder arg1) {
            ftduinoService = ((FtduinoService.UsbBinder) arg1).getService();
            ftduinoService.setHandler(mHandler);
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            ftduinoService = null;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mHandler = new MyHandler(this);
        display = (TextView) findViewById(R.id.info);

        // add handler to the switch
        switchbutton = (Switch) findViewById(R.id.switch_o1);
        switchbutton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
              if (ftduinoService != null) {
                    if(switchbutton.isChecked())
                        ftduinoService.write("ON\n".getBytes());
                    else
                        ftduinoService.write("OFF\n".getBytes());
              }
            }
        });

        is_connected(this, false);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu m) {
        menu = m;  // save menu for later use
        menuContext = this;

        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, m);

        // start with correct icon
        m.getItem(0).setIcon(ContextCompat.getDrawable(this,
                connected?R.drawable.ic_usb_connected:R.drawable.ic_usb_disconnected));

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        if ((id == R.id.action_usb) && !connected) {
            AlertDialog alertDialog = new AlertDialog.Builder(MainActivity.this).create();
            alertDialog.setTitle("No ftDuino connected");
            alertDialog.setMessage("Please connect your ftDuino to the USB port of this device using an USB OTG cable.");
            alertDialog.show();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onResume() {
        super.onResume();
        setFilters();  // Start listening notifications from FtduinoService
        startService(FtduinoService.class, usbConnection, null); // Start FtduinoService(if it was not started before) and Bind it
    }

    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mUsbReceiver);
        unbindService(usbConnection);
    }

    private void startService(Class<?> service, ServiceConnection serviceConnection, Bundle extras) {
        if (!FtduinoService.SERVICE_CONNECTED) {
            Intent startService = new Intent(this, service);
            if (extras != null && !extras.isEmpty()) {
                Set<String> keys = extras.keySet();
                for (String key : keys) {
                    String extra = extras.getString(key);
                    startService.putExtra(key, extra);
                }
            }
            startService(startService);
        }
        Intent bindingIntent = new Intent(this, service);
        bindService(bindingIntent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    private void setFilters() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(FtduinoService.ACTION_USB_PERMISSION_GRANTED);
        filter.addAction(FtduinoService.ACTION_NO_USB);
        filter.addAction(FtduinoService.ACTION_USB_DISCONNECTED);
        filter.addAction(FtduinoService.ACTION_USB_NOT_SUPPORTED);
        filter.addAction(FtduinoService.ACTION_USB_PERMISSION_NOT_GRANTED);
        registerReceiver(mUsbReceiver, filter);
    }

    /*
     * This handler will be passed to FtduinoService. Data received from serial port is displayed through this handler
     */
    private static class MyHandler extends Handler {
        private final WeakReference<MainActivity> mActivity;

        public MyHandler(MainActivity activity) {
            mActivity = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case FtduinoService.MESSAGE_FROM_SERIAL_PORT:
                    String data = (String) msg.obj;
                    mActivity.get().display.append(data);
                    break;
                case FtduinoService.CTS_CHANGE:
                    Toast.makeText(mActivity.get(), "CTS_CHANGE",Toast.LENGTH_LONG).show();
                    break;
                case FtduinoService.DSR_CHANGE:
                    Toast.makeText(mActivity.get(), "DSR_CHANGE",Toast.LENGTH_LONG).show();
                    break;
            }
        }
    }
}