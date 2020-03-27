
=====================
Laundry symbol reader
=====================


compile:
$ [make clean &&] make

install:
$ sudo make install

run (docker):
$ ./laundry-symbol-reader-dk relative/path/to/image.extension

run (non docker):
$ export IMG_FNAME=relative/path/to/image.extension
$ laundry-symbol-reader 
