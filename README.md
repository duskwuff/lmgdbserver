lmgdbserver
===========

A tiny GDB server for the TI Stellaris LaunchPad.


Requirements
------------

* libusb 1.0 or later.


Usage
-----

...is simple:

    (gdb) target remote | lmgdbserver

If you somehow have a LaunchPad with a nonstandard VID/PID or endpoints, you
can run `lmgdbserver -h` for the necessary details.


Known bugs
----------

* Hardware breakpoints don't seem to work. This ends up breaking a number of
  other functions as well, including `finish`.

  As far as I can tell, this is inherent to the GDB stub in the device, so
  there may not be very much I can do about it.

* Quitting GDB will leave the device in a stopped state. You'll have to power
  cycle the target to get it working normally again.


License
-------

Copyright (c) 2012 Dusk Wuff (dusk@woofle.net).

To the extent possible under law, the author has dedicated all copyright and
related and neighboring rights to this software to the public domain worldwide.
This software is distributed without any warranty.

