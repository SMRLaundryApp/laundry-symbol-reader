
=====================
Laundry symbol reader
=====================


Installing prerequisites:
=========================

The program depends on libalx-base and libalx-cv (both from libalx_) being installed in the system, and those libraries depend on some other packages being installed in the system.  For those reasons, I recommend debian 11 (bullseye) as the operating system.  That's the OS used in the docker image.  Other compatible OSes include: manjaro.

.. _libalx: https://github.com/alejandro-colomar/libalx

Installing prerequisites on debian 11:
--------------------------------------

.. code-block:: sh

	## install build tools:
	$ sudo apt-get install gcc g++ make git pkg-config

	## install libraries which libalx depends on:
	$ sudo apt-get install libbsd-dev libgsl-dev libopencv-dev

	## download libalx
	$ git clone https://github.com/alejandro-colomar/libalx.git

	## compile libalx
	$ make base cv -C libalx -j8

	## install libalx
	$ sudo make install-base install-cv -C libalx -j2


Running the program:
====================

compile:
--------

Before compiling you may want to modify the value of the variable ``dbg`` in the file ``src/dbg.h`` to some higher value to see some images and text of the process.  The value should be between 0 and 4, (values lower than 0 will act as 0, and values greater than 4 will act as 4).  The higher the value, the more debugging information.

.. code-block:: sh

	$ [make clean &&] make

install:
--------
.. code-block:: sh

	$ sudo make install

run (docker):
-------------
.. code-block:: sh

	$ ./laundry-symbol-reader-dk relative/path/to/image.extension

run (non docker):
-----------------
.. code-block:: sh

	$ export IMG_FNAME=relative/path/to/image.extension
	$ laundry-symbol-reader 
