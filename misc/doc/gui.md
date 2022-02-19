

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
To be completed and coming soon...

### Overview

----
### Port view

----
### Portgroup setup

**Input properties**

**Output properties**