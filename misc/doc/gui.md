

## Navigating the Menu
The rotary encoder is used for all operations within the graphical user interface. With it the user can perform 4 distinctive inputs:
| Input              | Action
| ------------------ | ---------------------------------- |
| Clockwise rotation | Select next item / increment value |
| Counter-clockwise rotation | Select previous item / decrement value
| Short button press (< 0.5 seconds pressed) | Confirm / choose selection
| Long button press (at least 0.5 seconds pressed) | Cancel / return to previous screen

----
## Menu Structure
The menu is structured as follows:
```
                                        -------------
                    --------------------| Main Menu |----------------
                    V                   -------------               V
    ------------------------------                          -------------------           ---------------------
    | Overview                   |                          | Portgroup setup |---------->| Add new portgroup |
    | (Default view on power-on) |                          -------------------           ---------------------
    ------------------------------                            |             |
                    |                                         |             |
                    |                                         V             V
                    V                   ---------------------------     ------------------------------
    ----------------------              | Change input properties |     | Change output properties   |
    | Port view (1 to 8) |              | of selected portgroup   |     | of selected portgroup      |
    ----------------------              | (Set MIDI channel,      |     | (Set demuxer, Add/remove   |
                    |                   | Add/remove MIDI input,  |     | output port, Set transpose |
                    |                   | Set controller)         |     | offset)                    |
                    V                   ---------------------------     ------------------------------
    --------------------------                          |                     |
    | Change port properties |                          |                     |
    | (Pitch/Velocity mode,  |                          V                     V
    | Clock Rate)            |          -----------------------         -----------------------
    --------------------------          | Value entry subview |         | Value entry subview |
                    |                   -----------------------         -----------------------
                    |
                    V
    -----------------------
    | Value entry subview |
    -----------------------
```

----
## Notable Views

### Overview
This is the standard view when powering on the module. It gives a quick summary of all ports and their activity. A asterisk character shows when a port is active or a change of the output voltage occurs. A number shows the current outputted analog value for each port. If the port outputs a MIDI note this represents the MIDI note number. A downwards arrow indicates the current selected port. Turning the rotary encoder moves the selection. Short pressing of the rotary encoder button switches to the port view of the selected port (see below). Holding and releasing the rotary encoder button switches the view up to the main menu.

----
### Port view
Port view shows current activity, the properties of the selected port and a menu to change the properties. These properties are:

**Output mode:**
Possible values are: "Note" or "Velocity".
Only significant when the port receives note messages. "Note" will have the port output pitch control voltage, "Velocity" outputs the velocity infomation of received note messages.

**Clock rate:**
Only significant when the port receives clock messages. Change the rate of clock triggers on the digital output portion in the range of 1/16th note (shortest, i.e. every 6 clock messages) and 1 per beat (currently longest, i.e. every 96 clock messages).

The menu item "Resync clock" resets the clock period manually so that it is in sync with the next clock message. Normally this is not needed as most MIDI equipment sends a Start or Continue message when MIDI clock is started or resumed which triggers the reset automatically.

----
### Portgroup setup
From the main menu one can select the portgroup setup. Then the first portgroup will be displayed. If no portgroups exist a view to add a new portgroup will be shown. Turn the rotary encoder to switch between the existing portgroups. One turn clockwise at the last portgroup will display the add portgroup screen also. The portgroups inputs will be shown on the left side, the outputs on the right side of the screen. The downwards arrow rests in the title initially indicating the current selected entry. Pressing the rotary encoder button will select the current portgroup and the arrow will move down. Now by turning and pressing the rotary encoder one can select either the input or output properties to modify them.

**Input properties**

Shown on the left half of the screen. The current selected channel in the first row. Below that a list of the assigned message types.
If the downwards arrow is in the left half of the screen a short button press on the rotary encoder will show a menu to modify the input properties.

***Channel:***
The MIDI channel the portgroup listens on. When listening for system messages (like clock) the channel is not relevant as system messages are channel independent.

***MIDI input types:***
MIDI message types the portgroup listens to.

***Continuous Controller:***
If cont. controller is added to the input types a controller number must be assigned in the next screen. Note that the selectable range spans from 0 to 31 and 64 to 127. The range of 32 to 63 is used by MIDI to transmit the least significant byte of the previous 32 controllers to permit a more fine control of continuous inputs like modulation wheels if the MIDI sender supports it.
The controller number can be changed later in the input properties menu of the portgroup.
If you want to use more than one cont. controller create a portgroup for each of them.

**Output properties**

Shown on the right half of the screen. The current selected demuxer in the first row. Below that a list of the assigned output ports.
If the downwards arrow is in the right half of the screen a short button press on the rotary encoder will show a menu to modify the output properties.

***Demuxer:***
The assigned demuxer.

***Output ports:***
The assigned ports with their number.

***Transpose:***
From the output properties menu a transpose offset can be set. A positive or negative amount of halftones added to all incoming note on/off messages which the assigned output ports will produce.