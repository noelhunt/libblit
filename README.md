# libblit
A merge of libj from jtools and libXg. This is a substitute for the Plan9 graphics available
in plan9port.

In 1988, Dave Kapilow and John Helfman released a 5620 emulator library for X11 and Suntools.
This library translates, in a sense, 5620 graphics routines into something equivalent in X11
or Suntools, as well as simulating the 5620 operating system wait/sleep/alarm calls. LibXg
was released by AT&T in the early 1990s and was a later version of the graphics orginally
done for the 5620, but it only deals with the X11 eventloop and X11 graphical operations.
I needed a rather more sophisticated X11 implementation of the 5620 graphics, but also needed
an emulation of the 5620 system itself (wait/sleep/alarm) so that pads could be built without
resorting to plan9port, so decided to merge the two.
