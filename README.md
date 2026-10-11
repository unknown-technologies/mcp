Emulight MCP
============

The Emulight MCP (Master Control Program) is a basic operating system for the
PDP-11/03 (LSI-11). The following features are supported at the moment:

- RX02 bootloader
- Filesystem
- Preemptive multitasking
- Relocating loader with dynamic linking


Subsystems
----------

Currently two subsystems are implemented:

- **SPF (Structured Panel Facility)**: this subsystem provides mechanisms to
  implement fullscreen panel based user interfaces.
- **TSO (Time Sharing Option)**: this subsystem provides a minimal command line
  interface inspired by the MVS TSO subsystem. Since the TSO subsystem does not
  load any panel related libraries, more RAM is available to user applications.

A small set of applications is implemented for both subsystems, which range
from tech demos to fully usable applications.


Build Requirements
------------------

- GNU make
- pdp11-aout-gcc, pdp11-aout-binutils
- [lsi-as](https://github.com/unknown-technologies/lsi-as)


Building
--------

To build the Emulight MCP, run the following command in the root of the
repository:

```
make
```

This command produces an RX02 disk image file called `disk.rx2`.


Booting the MCP
---------------

To boot the generated disk image file, you can either use simh or the
[lsi-11](https://github.com/hackyourlife/lsi-11) emulator.

Using lsi-11, run the following command:

```
lsi-11 -q -x -f disk.rx2
```

Using simh, run the following command:

```
./simh.sh
```


License
-------

This program is licensed under the terms of the GNU GPLv3.
