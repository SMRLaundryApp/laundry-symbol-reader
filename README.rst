
=====================
Laundry symbol reader
=====================


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
