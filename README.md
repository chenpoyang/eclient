compile
=======
1. cd test && make
2. make
3. make a symbol link to both libclient.so and libtrigger.so in '/usr/lib/' or '
/usr/local/lib/' eg:
    ln -s `pwd`/libeclient.so /usr/lib/libeclient.so
    ln -s `pwd`/libtrigger.so /usr/lib/libtrigger.so

usage
=====
1. cd test && ./server
2. ./emain
