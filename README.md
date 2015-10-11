Install
-------

Without nonfree:

    $ sudo apt-get install libopencv-dev

With nonfree:

http://efcan-bitsandpeices.blogspot.nl/2014/02/how-to-install-opencv-on-linux.html

    $ sudo apt-get remove libopencv-dev
    $ wget https://github.com/Itseez/opencv/archive/2.4.11.zip
    $ unzip 2.4.11.zip
    $ cd 2.4.11
    $ mkdir release
    $ cd release
    $ cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_opencv_nonfree=ON ..
    $ make
    $ sudo make install
    $ sudo ldconfig -v
