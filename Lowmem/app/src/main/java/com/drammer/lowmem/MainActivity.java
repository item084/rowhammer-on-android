package com.drammer.lowmem;

import static android.os.Process.SIGNAL_USR1;

import androidx.appcompat.app.AppCompatActivity;

import android.app.ActivityManager;
import android.content.Context;
import android.os.Bundle;
import android.os.Debug;
import android.os.Handler;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Field;

public class MainActivity extends AppCompatActivity {
    Process process;
    int pid;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Log.d("lowmemstart", "yay");


        Thread thread;
        final Handler handler = new Handler();
        thread =  new Thread(new Runnable() {
            public void run() {
                String myExec = "/data/data/com.drammer.lowmem/rh-attk";
                try {
                    File execFile = new File(myExec);
                    execFile.setExecutable(true);
                    process = Runtime.getRuntime().exec(myExec, null, null);
                } catch (java.io.IOException ex) {
                    Log.d("lowmemstart", "nah");
                    ex.printStackTrace();
                }
            }
        });
        thread.start();
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        Thread thread1;
        final Handler handler1 = new Handler();
        thread1 =  new Thread(new Runnable() {
            public void run() {
                pid = getPid(process);

                TextView txt = (TextView) findViewById(R.id.output);
                txt.setMovementMethod(new ScrollingMovementMethod());
                BufferedReader reader = new BufferedReader(
                        new InputStreamReader(process.getInputStream()));
                int read;
                char[] buffer = new char[4096];
                StringBuffer output = new StringBuffer();
                try {
                    // while ((read = reader.read(buffer)) > 0) {
                    while (true) {
                        Thread.sleep(100);
                        read = reader.read(buffer);
                        if (read < 0) {
                            continue;
                        }
                        output.append(buffer, 0, read);
                        Log.d("lowmemstart", output.toString());
                        runOnUiThread(new Runnable() {

                            @Override
                            public void run() {

                                // Stuff that updates the UI
                                txt.setText(output.toString());

                            }
                        });

                    }
                    // reader.close();
                } catch (IOException | InterruptedException ex) {}
            }
        });
        thread1.start();



        //try {
        //    process.waitFor();
        //} catch (java.lang.InterruptedException ex) {}
//        TextView myTextView = (TextView) findViewById(R.id.helloworld); //grab your tv
//        Runnable myRunnable = new Runnable() {
//            @Override
//            public void run() {
//                while (true) {
//                    try {
//                        Thread.sleep(10); // Waits for 1 second (1000 milliseconds)
//                        }
//                    catch (java.lang.InterruptedException ex) {
//                        }

//                    }
//                }
//            };
    }
    @Override
    public void onLowMemory() {
        super.onLowMemory();
        Log.d("lowmem", "Low Memory Low!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        Toast.makeText(MainActivity.this, (String) "low mem.",
                Toast.LENGTH_LONG).show();
    }

    public static int getPid(Process p) {
        int pid = -1;

        try {
            Field f = p.getClass().getDeclaredField("pid");
            f.setAccessible(true);
            pid = f.getInt(p);
            f.setAccessible(false);
        } catch (Throwable e) {
            pid = -1;
        }
        return pid;
    }
    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);
        android.os.Process.sendSignal(pid, SIGNAL_USR1);
        Log.d("lowmem", "Low Memory Trim!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"+Integer.toString(pid)+" "+level);



        // Called when the operating system has determined that it is a good
        // time for a process to trim unneeded memory from its process
    }
}

