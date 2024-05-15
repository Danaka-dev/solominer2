Bitcoin RPC
===========

A C++ wrapper library for JSON-RPC communication with Bitcoin and Bitcoin-like daemon.

The library simplifies communication with the Bitcoin daemon by taking care of packing and unpacking the various daemon JSON-RPC messages.

Building the library
--------------------

[![badge](https://img.shields.io/badge/Language-C++-green)]()
[![badge](https://img.shields.io/badge/release-v1.1.0-blue)]()
[![badge](https://img.shields.io/badge/License-MIT-blue)](https://github.com/Danaka-dev/bitcoin-rpc/blob/master/LICENSE.md)

**Dependencies**

This library requires [CMake](http://www.cmake.org/cmake/resources/software.html), [Curl](http://curl.haxx.se/libcurl/) and [libjson-cpp](https://github.com/open-source-parsers/jsoncpp) packages to be built. They may be installed on Linux as follows:

```sh
sudo apt-get install cmake libcurl4-openssl-dev libjsoncpp-dev
```

**Build and install**

Navigate to the root directory of the library and proceed as follows:

```sh
mkdir build
cd build
cmake ..
make
```

Using the library
-----------------
Instantiate class BitcoinRPC to connect to and query a Bitcoin daemon, such as in the following example: 

```
#include <bitcoin-rpc.h>

int main()
{
    std::string username = "user";
    std::string password = "pass";
    std::string address = "127.0.0.1";
    int port = 8332;

    try
    {
        /* Constructor to connect to the bitcoin daemon */
        BitcoinRPC btc( username ,password ,address ,port );

        /* Example method - getbalance to get wallet balance */
        std::cout << "Wallet balance: " << btc.getbalance() << std::endl;
    }
    catch( BitcoinRpcException e )
    {
        std::cerr << e.getMessage() << std::endl;
    }
}
```

To compile your program with BitcoinRPC you will need to link it with the library:
```
g++ getbalance.cpp -lbitcoinrpc
```

Both shared and static library are available when building with cmake.

The full list of available API calls can be found [here](https://en.bitcoin.it/wiki/Original_Bitcoin_client/API_calls_list). Nearly the complete list of calls is implemented and thoroughly tested.

License
-------

The bitcoin-api-cpp library is released under the terms of [MIT](http://en.wikipedia.org/wiki/MIT_License).

```
Copyright (c) 2014 Krzysztof Okupski
Copyright (c) 2024 Danaka developers

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
OR OTHER DEALINGS IN THE SOFTWARE.
```

Used libraries
--------------
- [Curl](http://curl.haxx.se/libcurl/) and [libjson-cpp](https://github.com/open-source-parsers/jsoncpp) for http communication and json manipulation respectively.

Acknowledgements
----------------
The original idea for a C++ wrapper for JSON-RPC communication originates from [here](https://github.com/mmgrant73/bitcoinapi) and was rebuilt from bottom up by Krzysztof Okupski [here](https://github.com/minium/bitcoin-api-cpp).
Danaka imported within the project the few bits of code used from [libjson-rpc-cpp](https://github.com/cinemast/libjson-rpc-cpp) thus removing this dependency. 