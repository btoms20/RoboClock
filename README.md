# RoboClock

Turning the [EEZYbotARM](https://www.thingiverse.com/thing:1015238) into a time telling machine! 

![RoboClock_Timelapse](/Images/RoboClock_600_30.gif)

### Overview
This project attempts to use the [EEZYbotARM](https://www.thingiverse.com/thing:1015238) designed by [daGHIZmo](https://www.thingiverse.com/daGHIZmo) ([Original License](https://creativecommons.org/licenses/by-nc/4.0/legalcode)) to keep time by arranging a set of 3D printed numbers in the correct order.

The clock consists of three parts...
*  A Podium
    *  A stand that supports the digits used to tell the current time
*  An Inventory
    *  A chainlink / sprocket / tray system that stores all of the excess digits
    *  See Inventory.h for the arduino code
    *  See /Parts/Inventory for parts (stl) or [Thingiverse](https://www.thingiverse.com/thing:4457850/files)
*  The EEZYbot Robot Arm (see Arm.h)
    *  Responsible for moving the physical digits between the inventory and podium
    *  See Arm.h for the arduino code
    *  See [Thingiverse](https://www.thingiverse.com/thing:1015238) for parts
    *  With modified gripper assembly at 
       *  /Parts/GripperModified
       *  or [Thingiverse](https://www.thingiverse.com/thing:4457850)

Every 5 minutes the arm rearranges/replaces the numbers on the podium to reflect the current time. Depending on how many numbers need changing, the entire update cycle can last anywhere from 45 to 90 secs.

### How To
Check out the [instructables for a detailed guide](https://www.instructables.com/id/RoboClock/) to building your own
*  Download, Slice, Print and Assemble the [EEZYbotARM](https://www.thingiverse.com/thing:1015238) 
*  Download, Slice, Print and Assemble the [RoboClock](https://www.thingiverse.com/thing:4457850) parts
*  Pro Tip 💡: Before printing the entire 18 segment chain, print a few segments of the chain and adjust the height of Chain_Spacer.stl to make sure your chain has the desired tolerances. It should rotate freely with minimal lateral sag.
*  Mount parts to surface
*  Upload the meastro_settings.txt to the Micro Maestro using the Maestro software from Pololu
*  Tweak the sequences if needed to ensure the pre-programmed sequences move the to the correct locations
*  Wire up the electronics / circuit
*  Download the arduino sketch and dependencies using the IDE's library manager
*  Enter your wifi's credentials in arduino_secrets.h (if using the MKR1000 Wifi)
*  Compile & Upload the code to the arduino
*  Arrange the numbers on the chain in the anti-clockwise order (0,0,1,1,1,2,2,3,3,4,4,5,5,5,6,7,8,9)
*  Place the remaining zero on the last podium slot ( _ , _ , _ , 0 ) 
*  Plug in and watch it probably not work...
*  Tune the maestro sequences until things work 

### Arduino SketchBooks
*  [RoboClock_Seeeduino](https://gitlab.com/btoms20/robotclock/-/tree/master/clock_seeeduino) (Tested and working)
    *  Includes 3 custom libraries
        *  Clock.h
            *  Main orchestrator between Inventory and Arm
            *  Responsible for determining operations to acheive a certain clock state
            *  Repsonsible for the synchronous execution of those operations to achive the desired clock state
        * Arm.h
            *  Responsible for running pre-recorded subroutines on the Micro Maestro
        * Inventory.h
            *  Responsible for presenting requested digits (or empty slots) via the stepper motor diver.
            *  Responsible for maintaining an inventory state (what digit resides in each slot).
*  [RoboClock_MKR1000](https://gitlab.com/btoms20/robotclock/-/tree/master/clock_mkr1000) (Tested and working)
    *  Uses the Arduino MKR1000 Wifi
    *  The same Clock, Arm and Inventory libraries as above with the addition of networked time sync via NTP
    *  Also includes a few simple functions to convert UTC to your timezone (while protecting for daylight savings time when appropriate) (not fully tested)
    *  Two operation modes (set the ```Inventory.usingSensors``` bool flag appropriately)
        *  Using Hall Effect Chain Sensors (closes the inventory loop to create a much more robust inventory system)
        *  Not using Chain Sensors (the chain system may bind and cause the stepper motor to lose steps, at which point the system can't recover)

### Arduino Libraries / Dependencies
*  [PololuMaestro](https://github.com/pololu/maestro-arduino)
*  [CheapStepper](https://github.com/tyhenry/CheapStepper)
*  Seeeduino Specific
    *  [Time & TimeLib](https://github.com/PaulStoffregen)
*  MKR1000 Specific
    *  [RTCZero](https://github.com/arduino-libraries/RTCZero/blob/master/src/RTCZero.h)

### Hardware ( ~150 USD if you need to purchase everything listed )
*  Pololu Servo Controller ([Micro Maestro](https://www.pololu.com/product/1350)) (requires 5v serial)
*  4x MG90S Servos
*  28BYJ-48 5V DC Stepper Motor 
*  ULN2003A Stepper Driver
*  MKR1000 or Seeeduino rev2.21 (used for its 5v gpio pins)
*  Voltage Level Shifter (if using 3.3v board like the Arduino MKR1000 Wifi)
*  5v ~3 Amp Power Supply
*  608ZZ Bearing
*  Assorted M3-M5 Nuts and Bolts (lock nuts recommended)
*  Jumper Wires and Breadboard
*  3x1mm Magnets
*  A3144 Hall Effect Sensors (optional but recommended)

### Tools
*  3D Printer
*  Windows Comp with USB for Pololu Meastro Software 
*  Arduino IDE
*  M3-M5 Allen Keys
*  Pliers
*  Wood Screws and Screw Driver
*  A surface to mount everything to (a recycled bookshelf works well)
*  Superglue
If using the chain sensor
*  Soldering Iron
*  2x Capacitors (0.1 uF)
*  2x Resistors (10k Ohms)

### Electronics / Circuit Diagram
![RoboClock_Circuit](/Images/RoboClock_Circuit.png)

### Variants
* [x]  Seeeduino Controller (done)
    *  Pros:
        *  Makes development simple. Has 5v serial that makes interfacing with the micro maestro easy.
    *  Downsides 
        * No network for time sync / management 
        * Limited RAM (we're currently at ~85% memory and getting warnings on the Arduino IDE)
* [x]  Arduino MKR1000 (~~waiting for logic level shifter~~)
    *  Pros:
        *  Wifi built in for time sync / management
        *  Faster & more RAM
    *  Cons:
        *  3.3v architecture requires the use of a 3.3v <-> 5v Logic Level Shifter for communication with the Micro Maestro
* [ ]  TODO: Raspberry Pi Zero W (not started, 3.3v gpio as well)


### Notes & References
*  Arduino 
    * [Writing a Library](https://www.arduino.cc/en/Hacking/LibraryTutorial)  
*  MKR1000
    *  [Additional Serial Ports - Example Code](https://github.com/guywithaview/Arduino-Test/blob/master/sercom/sercom.ino)
    *  [Additional Serial Ports - Forum](https://forum.arduino.cc/index.php?topic=393296.0)
    *  [WiFiRTC Example](https://www.arduino.cc/en/Tutorial/WiFiRTC)
*  [Micro Maestro Documentation](https://www.pololu.com/product/1350)
*  [Unipolar -> Bipolar 28BYJ Conversion](https://coeleveld.com/wp-content/uploads/2016/10/Modifying-a-28BYJ-48-step-motor-from-unipolar-to-bipolar.pdf)
*  Raspberry Pi
    *  [28BYJ Stepper Motor & ULN2003 Driver](https://learn.adafruit.com/adafruits-raspberry-pi-lesson-10-stepper-motors/overview)  

### 3D Printed Parts
*  [EEZYbotARM MRK 1](https://www.thingiverse.com/thing:1015238) designed by [daGHIZmo](https://www.thingiverse.com/daGHIZmo) ([Original License](https://creativecommons.org/licenses/by-nc/4.0/legalcode))
*  [RoboClock](https://www.thingiverse.com/thing:4457850)
