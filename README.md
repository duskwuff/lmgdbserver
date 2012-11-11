lmgdbserver
===========

A tiny GDB server for the TI Stellaris LaunchPad.


Usage
-----

...is simple:

    (gdb) target remote | lmgdbserver

If you somehow have a LaunchPad with a nonstandard VID/PID or endpoints, you
can run `lmgdbserver -h` for the necessary arguments.


Requirements
------------

* libusb 1.0 or later.


License
-------

Copyright (c) 2012 Dusk Wuff (dusk@woofle.net).

To the extent possible under law, the author has dedicated all copyright and
related and neighboring rights to this software to the public domain worldwide.
This software is distributed without any warranty.

