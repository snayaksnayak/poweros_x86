poweros_x86
===========

An experimental OS called PowerOS for the x86 platform

Summary:
========

This directory contains the source code for a experimental OS called PowerOS,
which I ported to the Raspberry PI as an educational OS. I now ported it back to x86,
because on my long business trips im in lack of a rasperry pi development stack.
It is partly inspired by the good old AmigaOS, but it is not compatible. Some
Functions are the same others dont. So dont rely on it.

Status:
=======
At the moment im reporting the Raspberry Pi Source back to x86. The system boots up at the moment,
shows debug output, scans the PCI bus and initializes timer.device and a mouseport.device. So nothing
special at the moment.

Directory Hierarchy:
====================

* root/src/kickstart -> Here you find all files that are needed to "kickstart" the system. 
* root/makefile -> makefile for the project
* root/run.sh -> Run script for QEMU to boot up the kernel

License:
========
While chekcing which license fits best, take this as a note:
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 - The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
