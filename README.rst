
=====================
Laundry symbol reader
=====================


Prerequisites
=============

The program depends on libalx-base and libalx-cv (both from libalx_) being
installed in the system, and those libraries depend on some other packages
being installed in the system.  For those reasons, I recommend debian 11
(bullseye) as the operating system.  That's the OS used in the docker image.
Other compatible OSes include: Manjaro_, Ubuntu_ 20.04 (focal).

.. _libalx: https://github.com/alejandro-colomar/libalx
.. _Manjaro: https://manjaro.org/
.. _Ubuntu: http://cdimage.ubuntu.com/daily-live/current/

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

Before compiling you may want to modify the value of the macro ``dbg`` in the
file ``src/dbg.h`` to some higher value to see some images and text of the
process.  The value should be between 0 and 4, (values lower than 0 will act as
0, and values greater than 4 will act as 4).  The higher the value, the more
debugging information.

.. code-block:: sh

	## download the code:
	$ git clone https://github.com/SMRLaundryApp/laundry-symbol-reader.git

	## optional: clean the repository
	$ make clean

	## compile
	$ make -j8

	## install
	$ sudo make install -j2

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

download:
---------

.. code-block:: sh

	$ docker image pull laundrysymbolreader/reader

run:
----

There are two options:  you can run the docker command directly, or you can
run a script included in this repository (you need to download the script (or
the full repository) for the second option).

docker:
.......

.. code-block:: sh

	$ path=path/to/image
	$ dir=$(realpath $(dirname ${path}))
	$ fname=$(basename ${path})
	$ docker container run --tty --interactive			\
		--volume ${dir}:${dir}					\
		--env IMG_FNAME=${dir}/${fname}				\
		laundrysymbolreader/reader

script:
.......

.. code-block:: sh

	## download the script:
	$ wget https://raw.githubusercontent.com/SMRLaundryApp/laundry-symbol-reader/master/laundry-symbol-reader-dk
	## Allow executing file as program:
	$ chmod +x laundry-symbol-reader-dk

	## run the script:
	$ ./laundry-symbol-reader-dk path/to/image
