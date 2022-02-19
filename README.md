# Midimagic
# A feature rich MIDI to CV module for eurorack
Midimagic is a open source MIDI to CV interface
designed for eurorack modular synthesizers to provide great flexibility in receiving, mapping and combining MIDI messages.
The (non-exhaustive) feature list includes:
- Compatible with eurorack synthesizer electrical standards (+/-12V power supply, 1V/oct tracking)
- 8 independent analog output ports (each with a variable voltage and trigger/gate output)
- Completely free mapping and combining of MIDI message types to the outputs
- Supports NoteOn/Off, Mono/polyphonic aftertouch, Control change, Pitch bend and MIDI clock messages
- MIDI message auto-learn
- Transposing of notes
- Hardware MIDI thru output
- Saving and recalling of the setup (Work in progress)

Midimagic is still under development so features, behavior or hardware may change.

----
## Overview
The core of Midimagic is a STM32f103 powered Bluepill board driving 2 AD57x4 quad-channel DACs for the control voltage part of each output port. Each of these 8 CV outputs is paired with a separate gate/trigger output. Those pairs we call (output) ports.
Interfacing with humans occurs via a rotary encoder with push button functionality and a monochrome OLED display.
There's a MIDI in socket as well as hardware based MIDI thru.
USB MIDI might be added in the future.

Midimagics software is written in C++ on top of the Arduino framework using the [Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) by Francois Best and [LCDGFX](https://github.com/lexus2k/lcdgfx) by Aleksei Dynda.

----
## Core Concept - Portgroups
Midimagic implements portgroups which provide the linking between
one or more MIDI message types on one MIDI channel and one or more output ports.
The number of portgroups, associated input message types and output ports is not limited.
It is also possible to have overlapping input types and output ports over multiple portgroups.
This provides the flexibility to adapt to (possibly) every usefull setup.
Each portgroup has 2 fixed (but changeable) properties:
- The MIDI channel it listens on
- A method of distributing incoming messages to output ports, called demuxer

```
----------------------------     ----------------------------     ---------------------------
|Set of MIDI message types |====>| Portgroup      | Demuxer =====>| Set of output ports     |
|                          |     | Channel N      |         |     |                         |
|                          |     |                ----------|     |                         |
|                          |     |                          |     |                         |
----------------------------     ----------------------------     ---------------------------
```

----
## Details of Operation
Take a look at the [documentation](misc/doc/using_midimagic.md) if you are interested in the workings of Midimagic.

----
## Building your own
The software and hardware specification of Midimagic is free to use for everyone.

See our [notes](misc/doc/building.md) and the [schematics](misc/schematics/) about making it yourself.

----
## Who we be - Raumschiffgeräusche
Raumschiffgeräusche ("Spaceship noises") is represented by Lukas Jünger([aut0](https://github.com/aut0)) and Adrian Krause

----
## License
This project is licensed under the GNU Lesser General Public License Version 3 - see the
[LICENSE](LICENSE) file for details.