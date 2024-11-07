# Volley VESC
This is the initial build of the Volley VESC.  Version 5.03 firmware was used as a baseline with the following changes:
 - Added Volley specific board files with our defaults defined.  These default values are loaded each time the VESC is power-cycled.
 - The shunts have been changed from 1 to 5 milliohms
 - CAN IDs are assigned at start up based on the Hardware ID stuffing:
    - Top: PC3 = 0, PC1 = 0 (CAN ID = 10d)
	- Left: PC3 = 0, PC1 = 1 (CAN ID = 11d)
	- Right: PC3 = 1, PC1 = 0 (CAN ID = 12d)
	- Unused: PC3 = 1, PC1 = 1 (CAN ID = 13d)
 - Motor direction is now configured based on the CAN ID.
 - `SENSE1` and `SENSE3` have been swapped to match the new hardware layout.
