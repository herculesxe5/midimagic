Midimagic is envisioned to be a central entry point for digital controlling devices communicating via MIDI (e.g. MIDI keyboards, sequencers, DAWs and such) to a analog synthesizer setup.

This document explains the behaviour of Midimagic and how you can use it in your setup. A basic knowledge of the MIDI protocol and it's concepts is expected from the reader. See [MIDI-Wikipedia](https://en.wikipedia.org/wiki/MIDI)

For a guide about the user interface see [here](gui.md).

## Details of Operation
### On the Matter of MIDI Messages
Until today Midimagic supports the following message types:
- Note Off
- Note On
- Polyphonic Aftertouch
- Control Change
- Monophonic/Channel Aftertouch
- Pitch Bend
- MIDI Clock/Start/Continue/Stop

More System message types might be added in the future.

See [Ports](#on-the-matter-of-ports) for more information about the behaviour of ports when receiving messages.

----
### On the Matter of Portgroups
Each portgroup operates on a single MIDI channel and distributes incoming messages via a demuxer to associated output ports.
Portgroups can have overlapping sets of input types and output ports, for example:

Portgroup 1 listens for NoteOn and NoteOff messages and sends them to the ports 1 to 3.

Portgroup 2 listens for Pitch Bend messages and sends them to the ports 1 to 4.

Using this setup one would have 1V/octave pitch control voltage combined with the pitch bend shift on ports 1 to 3 and
pitch bend CV only on port 4 for other uses.

More on the interpretation of different message types in the section [Ports](#on-the-matter-of-ports).
See [Examples](#examples) for more possible use cases.

When setup to receive Control Change messages an additional property per Portgroup is used: One MIDI controller to listen to

----
### On the Matter of Demuxers
A Demuxer distributes messages to ports following one of 3 available methods:

- **Random**

1 port is randomly picked

- **Identic**

Every message on all ports

- **FIFO**

The first free/inactive port is picked,
if all ports are currently assigned/active the "oldest"/longest active one is chosen.

This is usefull for playing polyphonically with multiple oscillators for example.

----
### On the Matter of Ports
Each port consists of a variable voltage and a gate/trigger output. The variable/control voltage (analog) output has a range of +/-5V and is used for continuous voltage such as pitch CV. The gate/trigger is a digital (low/high) output to indicate a change (trigger) of the continuous CV value output or gate representing a pressed key for example. Available voltages levels are either 0V or 10V.
The 2 outputs are used diffently depending on the type of received message.
Notable features are note/velocity and clock mode. Each of the 8 ports can be switched between outputting the pitch or velocity of received Note On messages at the analog output. Additionally each port can be switched between 5 clock modes, changing the behaviour when receiving Timing Clock and Start/Continue/Stop messages (see below).

The different MIDI message types are interpreted as follows:
| Type | Hex value | Analog output | Digital output |
| ---- | --------- | ------------- | -------------- |
| Note On | 0x08 | The pitch information as halftone offset from C4 with 1V/Oct scaling Or note velocity | High voltage until Note Off for the note is received |
| Note Off | 0x09 | unchanged | Low voltage |
| Polyphonic Key Pressure/Aftertouch | 0xA | Raw value scaled to ~5V | unchanged |
| Control Change | 0xB | Raw value | Low if value is less than 64, else High (as per MIDI spec) |
| Program Change | 0xC | Ignored * | Ignored * |
| Channel Pressure/Monophonic Aftertouch | 0xD | Raw value scaled to ~5V | unchanged |
| Pitch Bend | 0xE | Raw value scaled to 2 halftones (as per MIDI spec), but is added to the current value if port is active | unchanged |
| Clock | 0xF8 | unchanged | In sync-mode: Rate adjustable trigger with 5 clock ticks duty cycle<br>Other modes: unchanged |
| Start | 0xFA | unchanged | In sync-mode: Resyncs clock period on the next clock message.<br>In gate-mode: High voltage<br>In trigger-start-mode: High voltage<br>In trigger-continue and trigger-stop-mode: Low Voltage |
| Continue | 0xFB | unchanged | In sync-mode: Resyncs clock period on the next clock message.<br>In gate-mode: High voltage<br>In trigger-continue-mode: High voltage<br>In trigger-start and trigger-stop-mode: Low Voltage |
| Stop | 0xFC | unchanged | In trigger-stop-mode: High voltage<br>All other modes: Low voltage |

*Program Change messages are supposed to control the MIDI receiver, Midimagic itself in this case. As there is no concept of how Midimagic is supposed to react to these types they are ignored for now. Do you have an idea or suggestion as to how to control Midimagic via Program changes? Let us know!


#### Clock Modes
The different clock modes set the behaviour of the gate/trigger output when receiving Timing Clock and Start/Continue/Stop messages. These are most usefull to control sequencers.

- **Sync**
Rate adjustable trigger with 5 clock ticks duty cycle synced to the MIDI Timing Clock messages.
The available clock rates are: 1 trigger per beat/every 96 clocks, 1/2 notes/every 48 clocks, 1/4 notes/every 24 clocks, 1/8 notes/every 12 clocks and 1/16 notes/every 6 clocks.

- **Gate**
High if Start or Continue was received, low voltage on receipt of a Stop message. Basically signalling if the MIDI Clock is running or not. Usefull to start and stop a sequencer via a "run" input.

- **Start Trigger**
Sets the output high on receipt of a Start message. Can be used to reset a sequencer if a reset input is available. The next non-Start message will set the output low again.

- **Continue Trigger**
Sets the output high on receipt of a Continue message. The next non-Continue message will set the output low again.

- **Stop Trigger**
Sets the output high on receipt of a Stop message. The next non-Stop message will set the output low again.

----
## Using Midimagic in your Patch

### Examples
Here are a few suggestions of how to setup and work with Midimagic. Feel free to adapt and enhance these in your setup. All ports and MIDI channels noted are only exemplary and can be changed for your needs.

A simple monophonic setup:
```
----------------------------     ----------------------------     ---------------------------
| Note On                  |====>| Portgroup 1    | Demuxer =====>| Port 1                  |
| Note Off                 |     | Channel 1      | Identic |     | Port 2                  |
|                          |     |                ----------|     | Port 3                  |
|                          |     |                          |     | Port 4                  |
----------------------------     ----------------------------     ---------------------------
```

A simple polyphonic setup to control 3 VCOs:
```
----------------------------     ----------------------------     ---------------------------
| Note On                  |====>| Portgroup 1    | Demuxer =====>| Port 1                  |
| Note Off                 |     | Channel 1      | FIFO    |     | Port 2                  |
|                          |     |                ----------|     | Port 3                  |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
```

Combine Note messages from different MIDI channels to the same ports with Pitch Bend from channel 1:
```
----------------------------     ----------------------------     ---------------------------
| Note On                  |====>| Portgroup 1    | Demuxer =====>| Port 1                  |
| Note Off                 |     | Channel 1      | FIFO    |     | Port 2                  |
|                          |     |                ----------|     | Port 3                  |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
----------------------------     ----------------------------     ---------------------------
| Note On                  |====>| Portgroup 2    | Demuxer =====>| Port 1                  |
| Note Off                 |     | Channel 5      | FIFO    |     | Port 2                  |
|                          |     |                ----------|     | Port 3                  |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
----------------------------     ----------------------------     ---------------------------
| Pitch Bend               |====>| Portgroup 3    | Demuxer =====>| Port 1                  |
|                          |     | Channel 1      | Identic |     | Port 2                  |
|                          |     |                ----------|     | Port 3                  |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
```

Use Pitch and Velocity CV in parallel:
```
----------------------------     ----------------------------     ---------------------------
| Note On                  |====>| Portgroup 1    | Demuxer =====>| Port 1 (in pitch mode)  |
| Note Off                 |     | Channel 3      | FIFO    |     | Port 2 (in pitch mode)  |
|                          |     |                ----------|     | Port 3 (in pitch mode)  |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
----------------------------     ----------------------------     -----------------------------
| Note On                  |====>| Portgroup 2    | Demuxer =====>| Port 4 (in velocity mode) |
| Note Off                 |     | Channel 3      | FIFO    |     | Port 5 (in velocity mode) |
|                          |     |                ----------|     | Port 6 (in velocity mode) |
|                          |     |                          |     |                           |
----------------------------     ----------------------------     -----------------------------
```

Output Clock pulses and Monophonic Aftertouch parallel on the same port:

Aftertouch will change the analog output while Clock will change only the digital output
```
----------------------------     ----------------------------     ---------------------------
| Clock                    |====>| Portgroup 1    | Demuxer =====>| Port 8                  |
| Channel Pressure         |     | Channel 1      | Identic |     |                         |
|                          |     |                ----------|     |                         |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
```

Use different Controllers simultaneously:

As the purposes of MIDI Controllers are merely suggestions it is up to the user what is to be controlled by them.
E.g. Controller 7 could be controlled via a MIDI clip from a DAW to adjust the cutoff frequency of a VCF.
```
----------------------------     ----------------------------     ---------------------------
| Control Change           |====>| Portgroup 1    | Demuxer =====>| Port 5                  |
|                          |     | Channel 1      | Identic |     |                         |
|                          |     |                ----------|     |                         |
|                          |     | Controller 1 (Mod wheel) |     |                         |
----------------------------     ----------------------------     ---------------------------
----------------------------     ----------------------------     ---------------------------
| Control Change           |====>| Portgroup 2    | Demuxer =====>| Port 6                  |
|                          |     | Channel 1      | Identic |     |                         |
|                          |     |                ----------|     |                         |
|                          |     | Controller 7 (Volume)    |     |                         |
----------------------------     ----------------------------     ---------------------------
----------------------------     ----------------------------     ---------------------------
| Control Change           |====>| Portgroup 3    | Demuxer =====>| Port 7                  |
|                          |     | Channel 1      | Identic |     |                         |
|                          |     |                ----------|     |                         |
|                          |     | Controller 11(Expression)|     |                         |
----------------------------     ----------------------------     ---------------------------
```

**More experimental examples:**

Distribute Notes randomly but have Pitch Bend on all ports:
```
----------------------------     ----------------------------     ---------------------------
| Note On                  |====>| Portgroup 1    | Demuxer =====>| Port 1                  |
| Note Off                 |     | Channel 1      | Random  |     | Port 2                  |
|                          |     |                ----------|     | Port 3                  |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
----------------------------     ----------------------------     ---------------------------
| Pitch Bend               |====>| Portgroup 2    | Demuxer =====>| Port 1                  |
|                          |     | Channel 1      | Identic |     | Port 2                  |
|                          |     |                ----------|     | Port 3                  |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
```

Pitch CV and Clock as ADSR trigger on the same port (clock retriggers the notes like an arppegiator):
```
----------------------------     ----------------------------     ---------------------------
| Note On                  |====>| Portgroup 1    | Demuxer =====>| Port 5                  |
| Note Off                 |     | Channel 1      | Identic |     |                         |
| Clock                    |     |                ----------|     |                         |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
```

Use MIDI Clock as random triggers:
```
----------------------------     ----------------------------     ---------------------------
| Clock                    |====>| Portgroup 1    | Demuxer =====>| Port 5                  |
|                          |     | Channel 1      | Random  |     | Port 6                  |
|                          |     |                ----------|     | Port 7                  |
|                          |     |                          |     | Port 8                  |
----------------------------     ----------------------------     ---------------------------
```

As mentioned before any combination of message types or ports is possible.
