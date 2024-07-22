# SOLOMINER 2

[![badge](https://img.shields.io/badge/Language-C++-green)]()
[![badge](https://img.shields.io/badge/Release-v2.0.beta2-blue)]()
[![badge](https://img.shields.io/badge/Platform-Linux-orange)]()
[![badge](https://img.shields.io/badge/License-MIT-blue)](https://github.com/Danaka-dev/solominer2/blob/main/LICENSE)

**Howdy fellow solo adventurers and welcome to your very own one-stop, jack of all trades, swiss knife digital
currency mining assistant.**

![](https://github.com/Danaka-dev/solominer2/blob/main/assets/images/screenshot/solominer2-beta2.png)

**Solominer** lets you pilot optimized mining algorithms and manage the complex configuration and communication involved 
with mining digital currency from a straightforward, comfortable and great looking (or at least we think so) graphic user
interface.

### ![text](https://cdn.jsdelivr.net/gh/Readme-Workflows/Readme-Icons@main/icons/octicons/ApprovedChanges.svg) *features*

+ All the information to make the right mining decision at your fingertip\
\
    *How many blocks you will mine per day for each blockchain, what will be the earning per day, in $, in
BTC... it's all there.*


+ Mining directly on your local node in total solo mode\
\
*No fee, no centralization, all the fun. Several cpu mineable blockchains supported, use your existing own core 
daemon/wallet or the ones provided, all fully configurable from the comfort of the user interface.*


+ A nifty dashboard and mining monitor\
\
*Giving you a clear and concise view of your ongoing mining journey. Hashrate, part per second, luck, progress 
toward statistical completion of a block, block mined... all in a nice interface with a cool retro feel (hope you like
it as much as we do :)*


+ Switch between the digital currency you are mining with a click of a button \
\
*Start and stop core daemon or wallet application from within the dashboard, get a new address, select or switch the 
currency you are mining with a click or let solominer auto mode select the blockchain with the highest earnings
for you automatically.*   


+ Automatically trade your reward \
\
*Manually or automatically trade your mining reward to another currency such as BTC or USDT and track your earnings
and trading directly from the user interface.*
\
(WARNING: this is a non fully debugged BETA feature, only use for testing or development purpose at this time!)

### ![text](https://cdn.jsdelivr.net/gh/Readme-Workflows/Readme-Icons@main/icons/octicons/ApprovedChanges.svg) *version*

**Solominer** current version is **BETA 2**, which is an early access version, hopefully not too much riddled 
with bugs.

At this stage, we do not recommend to use the software for any serious mining operation, nor to use Solominer source
code as part of your project yet. (unless a passion for the aforementioned hymenoptera compels you to do so of course :-)

But do have a look, test the software, tell us what you think. Your feedback is important!


Running the software
-----------------------

![](https://github.com/Danaka-dev/solominer2/blob/main/assets/images/os/linux-16x16.png)
**Linux**

1. Download and run

Make sure to download the binaries and configuration files from the official solominer2 repository:\
https://github.com/Danaka-dev/solominer2 

Download the release files and check that the 'solominer' file is allowed to be executed as a program before
executing solominer as usual.

To enable the use of huge pages and advanced cpu configuration requires root privilege, from the terminal in your
binary's directory:

```sh
sudo ./solominer-beta2-x64-linux
```
***NOTE***
*there might be a delay before the gui appears as the program is gathering run-time information about the blockchains.*

2. Configure

Within the program, configure connections to your existing core/wallet, or start the ones provided with the release.

You can access the configuration dialog by clicking on the wallet balance ('not connected' if no connection yet) or 
clicking the label below the core icon (that's on the left of the start button) in the list of connections. 

Before mining make sure to input your own mining address. You can use the 'new' button to the right of the address edit
box in the configuration dialog to get a new address from your core/wallet, or input your address manually in the edit box.    

It is also possible to configure the connection by editing the solominer.conf file directly before starting the 
application.


***NOTE***
+ *when first starting a full core daemon or wallet it will synchronize with the blockchain, this can take a long time
  and may require over 1Gb of disk space per blockchain.*
+ *Solominer will need to mine for a few minutes before the statistical values, such as earnings per day, become well
  accurate for your machine.*
+ *precompiled core binaries may be compressed in the release directory, look into ./cores/ subdirectory for the
  the blockchain you want to use and uncompress any .zip files.*

![](https://github.com/Danaka-dev/solominer2/blob/main/assets/images/os/windows-16x16.png)
**Windows**

 *coming soon in a later version!*

Building the software
---------------------

+ **Make sure your system is up to date**
```sh
sudo apt-get update
```

+ **Get the source code**

You may download the source the code from [gitHub](https://github.com/Danaka-dev/solominer2) or clone the repository
using [Git](https://github.com/git-guides/install-git) :

```sh
sudo apt-get install git build-essential
git clone https://github.com/Danaka-dev/solominer2.git
```

If you already cloned the repository and simply want to update with the most recent code, from your local repository
directory: 
```sh
git pull
```

+ **Dependencies**

The application requires [CMake](http://www.cmake.org/cmake/resources/software.html), [Curl](http://curl.haxx.se/libcurl/), [libjson-cpp](https://github.com/open-source-parsers/jsoncpp) and other packages to be built. They may be installed on Linux as follows:

```sh
sudo apt-get install cmake
sudo apt-get instal libcurl4-openssl-dev libjsoncpp-dev libuv1-dev libssl-dev libhwloc-dev
sudo apt-get install libx11-dev libxcursor-dev zlib1g zlib1g-dev libpng-dev uuid-dev 
```
(NB: zlib1g is with a '1', not an 'i' :)

+ **Build and run (terminal)**

Navigate to solominer2 directory and proceed to build from the terminal as follows:

```sh
mkdir build
cd build
cmake ..
make -j8
```
*(NB: you may replace -j8 with the actual number of cores on your machine if different)*

After building, copy bin assets and conf and run the application from build/bin directory:
```sh
cp -r ../bin .
cd bin
sudo ./solominer
```
*At first start gui may take a bit of time to appear. See notes in section **'Running the software'** above for details
about running and configuring solominer*

+ **Build and run (IDE)**
 
If using CLion, Visual Code or other cmake compatible environment relying on vcpkg, install required vcpkg and add
the following cmake options in your settings:
```sh
"-DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake -DCMAKE_PAK=VCPKG"
```
(e.g. <vcpkg-root> will probably be "/home/<username>/.vcpkg-clion/" for CLion on Linux)

After the initial cloning and before debug/run, copy ./bin directory containing assets and conf files to ./build/bin
working directory.

+ **Using daemon/wallet/core**

Solominer will communicate with running daemon(s) or wallet(s) (such as MAXE,RTC,RTM...) to provide earnings information,
direct mining on your full node, and automatic mining reward exchange. Make sure your ip port, --rpcuser and --rpcpass 
is set and match core configuration in solominer.
\
You can also start and stop daemon/wallet from solominer which will configure this for you. By default solominer assumes
daemon or wallet is located in '.bin/cores/' directory, this can be changed in solominer core settings or in section
[CORES] from service.conf file.  

Contribute
----------
> <cite> Mining solo harbors this strange impression of togetherness and solidarity that arise from acting as an 
> individual, in our own bubble, while at the same time participating in the most positive way to a greater thing. </cite>

The **Solominer** adventure is that of community and is open for everyone to participate.

### *Many ways one can contribute to Solominer:* 

+ Using the software, participating in GitHub discussion and providing feedback.
+ Giving Solominer a shout-out, tell your friends, let the world know!
+ Report on bugs, isolate bugs, propose fixes.
+ Review code, write test units, fix typoes.
+ Write documentation, tutorials, samples.
+ Provide translation.
+ Propose features and improvements.
+ And more... 

### *Code of conduct*

A positive attitude and constructive mindset is all you need to participate to Solominer.

Coming soon
-------------
We are working torward the release of **Solominer** debugged and tested **RELEASE CANDIDATE (RC)** version, adding
support for more pools and blockchains on the way.
\
Windows/MSVC port will follow right after. 

### ***Stay tuned !***


Donation
--------

**Solominer** is configured for a 3% donation level, which can easily be changed or removed from the source code.
If you do so however, you might want to consider making a one-off donation to support our work. 

If you wish to fuel development of **Solominer** with a donation you are welcome to do so here :

[![badge](https://img.shields.io/badge/BTC-yellow)]() [![badge](https://img.shields.io/badge/bc1q3f6fc08q4vkr7ejswsyj9e0yf3w3jexcnt36hs-gray)](https://github.com/Danaka-dev/solominer2/blob/main/DONATION) \
[![badge](https://img.shields.io/badge/ETH-green)]() [![badge](https://img.shields.io/badge/0x83c9a0D5318C748093eFF8c5008dF33B43C12B00-gray)](https://github.com/Danaka-dev/solominer2/blob/main/DONATION) \
[![badge](https://img.shields.io/badge/LTC-lightblue)]() [![badge](https://img.shields.io/badge/LUpBCo8Q4UgAQs5stDXNnkJyXwj6o1DEhy-gray)](https://github.com/Danaka-dev/solominer2/blob/main/DONATION) \
[![badge](https://img.shields.io/badge/RVN-red)]() [![badge](https://img.shields.io/badge/RHLbhxGcZVyQngcDaXaVd1DzK1bTjw1hoQ-gray)](https://github.com/Danaka-dev/solominer2/blob/main/DONATION) \
[![badge](https://img.shields.io/badge/RTM-orange)]() [![badge](https://img.shields.io/badge/RHr8GtzukKvAKRWSmnHu3WnP8xoGRSYRjL-gray)](https://github.com/Danaka-dev/solominer2/blob/main/DONATION) 

Top donators will be featured in the credit of the software, if you wish, let us know the name and a custom text you
would like to appear in the software.

For donations related to feature request, drop us a note beforehand to avoid any disappointment as making a donation
does not guarantee we will choose to make the requested feature.

---

### *footnote*
We are dedicated to share great quality software and work hard to provide a stable, bug free and secure experience to
our users, but unfortunately cannot provide any warranty of any kind for using the software which you are using at your
own risk.

Software and cryptocurrency are complex worlds, which require everyone to stay informed and exercise good judgment.

In this regard make sure to only download Solominer and core, wallet or other software in particular those related to
cryptocurrency from secure site and reputable sources. Inspect and compile the source code yourself if you can, use
caution when using noncustodial cryptocurrency solutions, keep your passwords well secured preferably in cold storage.

---
![](https://github.com/Danaka-dev/solominer2/blob/main/assets/images/solominer-32x32.png)

### ***Thank you for using Solominer, and, as always, may the blocks be with you!***
[Danaka](mailto:danaka-dev@pm.me)
