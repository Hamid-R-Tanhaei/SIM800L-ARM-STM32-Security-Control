## A simple security/IoT project using SIM800L GSM module 
In this project a SIM800L module is driven by an ARM micro-controller STM32F103C8T6 to implement a simple controlling/security device communicating with a cell phone through GSM network. The device continuously monitors the peripheral sensors such as vision motion detectors or knock sensors, and sends alarms to the cell phone by SMS and calling. Moreover, as an option, the device can connect to a PC through USB (HID class) in which a Java application runs. The java app can be used to communicate to the SIM module by AT commands by user.

![overal block diagram](Extras/SIM800.jpg)

The device is provided by a Lithium battery and charging circuitry. So in case of input power outage the device is able to operate normally for hours but sends an alarm to the user cell phone. 


![board photo](Extras/IMG.jpg)

![screenshots](Extras/sim800_.jpg)
