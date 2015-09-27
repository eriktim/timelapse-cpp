Install
-------

Without nonfree:

    # apt-get install libopencv-dev

With nonfree:

    $ wget https://github.com/Itseez/opencv/archive/2.4.11.zip
    $ unzip 2.4.11.zip
    $ cd 2.4.11
    $ mkdir release
    $ cd release
    $ cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_opencv_nonfree=ON ..
    $ make
    # make install
    # ldconfig -v
