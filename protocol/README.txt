Those files come from a public project derived from the Latero work at Tactile Labs.

The files should be kept as-is and avoid any changes in the protocol to keep things compatible with other
work.

It is however possible to extend the protocol.  In such case, a new packet type should be created and that
packet type should be added the the latero_io.h file along with the unpack/pack methods.  Changes to the protocol
should be made public to help everyone.

https://gitlab.com/jerome.pasquero/latero/tree/master/latero/tl-latero

