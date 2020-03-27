
=====================
Laundry symbol reader
=====================

Before compiling you may want to modify the value of the variable ``dbg`` in the file ``src/dbg.h`` to some higher value to see some images and text of the process.  The value should be between 0 and 4, (values lower than 0 will act as 0, and values greater than 4 will act as 4).  The higher the value, the more debugging information.


compile:
--------
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
