/*
 * The MIT License
 *
 * Copyright 2019 Bitcoin Solutions Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
package org.hid4java;
import java.util.Iterator;
import java.util.Scanner;
import org.hid4java.event.HidServicesEvent;
import java.util.concurrent.TimeUnit;
import static java.util.concurrent.TimeUnit.NANOSECONDS;

/**
 *
 * @author Hamid
 */
public class RunHidDevice implements HidServicesListener {

  private static final Integer VENDOR_ID = 0x16c0; //0x534c;
  private static final Integer PRODUCT_ID = 0x05df; //0x01;
  private static final int PACKET_LENGTH = 64;
  public static final String SERIAL_NUMBER = null;

  public static void main(String[] args) throws HidException {

    RunHidDevice example = new RunHidDevice();
    example.executeExample();

  }

  public void executeExample() throws HidException {

    // Configure to use custom specification
    HidServicesSpecification hidServicesSpecification = new HidServicesSpecification();
    hidServicesSpecification.setAutoShutdown(true);
    hidServicesSpecification.setScanInterval(500);
    hidServicesSpecification.setPauseInterval(5000);
    hidServicesSpecification.setScanMode(ScanMode.SCAN_AT_FIXED_INTERVAL_WITH_PAUSE_AFTER_WRITE);

    // Get HID services using custom specification
    HidServices hidServices = HidManager.getHidServices(hidServicesSpecification);
    hidServices.addHidServicesListener(this);

    // Start the services
    System.out.println("Starting HID services.");
    hidServices.start();

    System.out.println("Enumerating attached devices...");

      // Provide a list of attached devices
      hidServices.getAttachedHidDevices().forEach((hidDevice) -> {
          System.out.println(hidDevice);
      });

    // Open the device device by Vendor ID and Product ID with wildcard serial number
    HidDevice hidDevice = hidServices.getHidDevice(VENDOR_ID, PRODUCT_ID, SERIAL_NUMBER);
    if (hidDevice != null) {
      // Consider overriding dropReportIdZero on Windows
      // if you see "The parameter is incorrect"
      // HidApi.dropReportIdZero = true;

      // Device is already attached and successfully opened so send message
      sendMessage(hidDevice);
    }

    System.out.printf("Waiting 3s to demonstrate attach/detach handling. Watch for slow response after write if configured.%n");

    // Stop the main thread to demonstrate attach and detach events
    sleepUninterruptibly(3, TimeUnit.SECONDS);

    // Shut down and rely on auto-shutdown hook to clear HidApi resources
    hidServices.stop();
    hidServices.shutdown();

  }

  @Override
  public void hidDeviceAttached(HidServicesEvent event) {

    System.out.println("Device attached: " + event);

    // Add serial number when more than one device with the same
    // vendor ID and product ID will be present at the same time
    if (event.getHidDevice().isVidPidSerial(VENDOR_ID, PRODUCT_ID, null)) {
      sendMessage(event.getHidDevice());
    }

  }

  @Override
  public void hidDeviceDetached(HidServicesEvent event) {

    System.err.println("Device detached: " + event);

  }

  @Override
  public void hidFailure(HidServicesEvent event) {

    System.err.println("HID failure: " + event);

  }

  private void sendMessage(HidDevice hidDevice) {

    // Ensure device is open after an attach/detach event
    if (!hidDevice.isOpen()) {
      hidDevice.open();
    }

    // Send the Initialise message
    byte[] message = new byte[PACKET_LENGTH];
    byte data[] = new byte[PACKET_LENGTH]; 
    message[0] = 0x00; // USB: Payload 63 bytes
    message[1] = 0x00; // Device: '#'
    message[2] = 0x00; // Device: '#'
    message[3] = 0x00; // INITIALISE
    int val;
     Scanner in = new Scanner(System.in);
    //int val = hidDevice.write(message, PACKET_LENGTH, (byte) 0x00);
    //if (val >= 0) {
    //  System.out.println("> [" + val + "]");
    //} else {
    //  System.err.println(hidDevice.getLastErrorMessage());
   // }

    // Prepare to read a single data packet
    boolean moreData = true;
    System.out.println("");
    System.out.println("");
    System.out.println("");
    System.out.println("Waiting for AT command.... (enter 'q' to exit)\n");
    //for(int i = 0; i < 10; i++) {
    while(moreData){
        message[0] = 0;
        System.out.print(">");
        String s = in.nextLine();
        if (s.length() != 0)
        {
            if (s.toLowerCase().charAt(0) == 'q')
            {
                moreData = false;
                break;
            }
            if (s.equals("sub")){
                message[1] = (char)26;
                for (int idx = 1; idx < (message.length - 1); idx++)    
                {
                    message[idx+1] = 0;
                }
            }
            for (int idx = 0; idx < (message.length - 1); idx++)    
            {
                if(idx < s.length())
                    message[idx+1] = (byte) s.charAt(idx);
                else
                    message[idx+1] = 0;
            }
            
        }
        else
        {
            for (int idx = 0; idx < (message.length); idx++)    
            {
                message[idx] = 0;
            }
        }
            val = hidDevice.write(message, PACKET_LENGTH, (byte) 0x00);
            if(val < 0){
                moreData = false;
                System.err.println(hidDevice.getLastErrorMessage());
            }    
        
        //if (message[1] > 10)
        //{ moreData = false;}
        //    wait(1000);

        //if (val >= 0) {
          //System.out.println("> [" + val + "]");
        //} 
    
        // s = in.nextLine();

      //byte data[] = new byte[PACKET_LENGTH];
          // This method will now block for 500ms or until data is read
          for (byte b : data) {
            b = 0;
          }
          
          val = hidDevice.read(data, 5000);
      switch (val) {
        case -1:
          System.err.println(hidDevice.getLastErrorMessage());
          moreData = false;
          break;
        case 0:
          moreData = false;
          break;
        default:
          //System.out.print("< [");
          for (byte b : data) {
          //  if (data[0] == 0)
            //System.out.printf(" %02x", b);
            System.out.print((char) b);
          }
          System.out.println();
          break;
      }
    if (moreData == false)
    { break;}
    /*else
    {
        if (i % 250 == 0)
        {        
            System.out.print(java.time.LocalTime.now());
            System.out.print("\t250 packets Sent...\t");
            System.out.print("250 packets Received...\n");
        }

    }*/
    //System.out.println(java.time.LocalTime.now());  
  }
  }
  /**
   * Invokes {@code unit.}{@link java.util.concurrent.TimeUnit#sleep(long) sleep(sleepFor)}
   * uninterruptibly.
   */
  public static void sleepUninterruptibly(long sleepFor, TimeUnit unit) {
    boolean interrupted = false;
    try {
      long remainingNanos = unit.toNanos(sleepFor);
      long end = System.nanoTime() + remainingNanos;
      while (true) {
        try {
          // TimeUnit.sleep() treats negative timeouts just like zero.
          NANOSECONDS.sleep(remainingNanos);
          return;
        } catch (InterruptedException e) {
          interrupted = true;
          remainingNanos = end - System.nanoTime();
        }
      }
    } finally {
      if (interrupted) {
        Thread.currentThread().interrupt();
      }
    }
  }

}
