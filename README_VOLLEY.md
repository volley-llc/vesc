# Volley VESC (V4.12 HW)
This is the initial build of the Volley VESC.  Version 6.05 firmware was used as a baseline with the following changes:
 - Added Volley specific board files with our application and motor defaults defined.  These default values are loaded each time the VESC is power-cycled.  Eeprom reads at boot are no longer supported.  
 - The shunts have been changed from 1 to 5 milliohms
 - CAN IDs are assigned at start up based on the Hardware ID stuffing:
    - Top: PC3 = 0, PC1 = 0 (CAN ID = 10d)
	- Left: PC3 = 0, PC1 = 1 (CAN ID = 11d)
	- Right: PC3 = 1, PC1 = 0 (CAN ID = 12d)
	- Unused: PC3 = 1, PC1 = 1 (CAN ID = 13d)
 - Motor direction is now configured based on the CAN ID.
 - `SENSE1` and `SENSE3` have been swapped to match the new hardware layout.
 - Motor tuning defaults are configured for a Moon's 80BL100L2 BLDC motor.