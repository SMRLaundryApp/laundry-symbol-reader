
=====================
Laundry symbol reader
=====================


Prerequisites
=============

The program depends on libalx-base and libalx-cv (both from libalx_) being
installed in the system, and those libraries depend on some other packages
being installed in the system.  For those reasons, I recommend
`debian 11 (bullseye)`_ as the operating system.  That's the OS used in the
`docker image`_.  Other compatible OSes include: Manjaro_,
`Ubuntu 20.04 (focal)`_.

.. _libalx:			https://github.com/alejandro-colomar/libalx
.. _`debian 11 (bullseye)`:	https://www.debian.org/devel/debian-installer/
.. _Manjaro:			https://manjaro.org/
.. _`Ubuntu 20.04 (focal)`:	http://cdimage.ubuntu.com/daily-live/current/
.. _`docker image`:	https://hub.docker.com/r/laundrysymbolreader/reader

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

Installing prerequisites on manjaro:
--------------------------------------

.. code-block:: sh

	## install build tools:
	$ sudo pacman -S gcc make git pkgconf
	## install libraries which libalx depends on:
	$ sudo pacman -S libbsd gsl opencv
	## download libalx
	$ git clone https://github.com/alejandro-colomar/libalx.git
	## compile libalx
	$ make base cv -C libalx -j8
	## install libalx
	$ sudo make install-base install-cv -C libalx -j2


Running the program
===================

compile:
--------

Before compiling you may want to modify the value of the macros ``DBG`` and
``DBG_SHOW_WAIT`` in the file ``src/dbg.h`` to some higher value to see some
images and text of the process, and to be able to stop the program at each
image.  The value should be between 0 and 4, (values lower than 0 will act as
0, and values greater than 4 will act as 4).  The higher the value, the more
debugging information.

.. code-block:: sh

	## download the code:
	$ git clone https://github.com/SMRLaundryApp/laundry-symbol-reader.git
	## compile
	$ make -C laundry-symbol-reader -j8
	## install
	$ sudo make install -C laundry-symbol-reader -j2

run:
----
.. code-block:: sh

	$ export IMG_FNAME=path/to/image
	$ laundry-symbol-reader 

Docker
======

If you don't have the system prerequisites above you can just run a docker
container with the program already installed.
This docker image is also used by the App.
The only drawback is that you don't have a display; the only output is text.
There is a script to run the docker container easily.

install:
--------

.. code-block:: sh

	## download the latest docker image
	$ docker image pull laundrysymbolreader/reader
	## clone the repository:
	$ git clone https://github.com/SMRLaundryApp/laundry-symbol-reader.git
	## install script
	$ sudo make inst-scripts -C laundry-symbol-reader
	## remove the repository clone:
	$ rm -rf laundry-symbol-reader

run:
----

.. code-block:: sh

	$ laundry-symbol-reader-dk path/to/image

