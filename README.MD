# Introduction
This software is based on the work of others. Here is the approximate timeline:
- First, the majority of the code and dependencies utilize the srsRAN project.
- Second, using srsRAN, (being called srsLTE at the time), Eckermann created a `cv2x_traffic_generator` tool. <!-- CITATION NEEDED -->
    - His contributions included custom `ue_sl` code that was buried in srsRAN's source and the `cv2x_traffic_generator.c`.
- Third, using Eckermann's work, Twardoku updated the code to work on the version of srsRAN current to him.

Taking Twardoku's source, I have updated the `ue_sl` to be once again compatible with the 2023 version of srsRAN. 

# Hardware requirements
- An SDR capable of running srsRAN. We used an EttusResearch USRP X410. 

# Prerequisites

Install UHD or other appropriate SDR driver software.

<!--TODO - Include the terminal commands and references for how to install srsRAN -->
Build and install srsRAN. The key detail is that srsRAN libraries (the `.so` and `.a` files) need to appear in `/usr/local/lib/` and srsRAN headers (the `.h` files) need to appear in `/usr/include/srsran/` because that is where my makefile looks for them.

# Compiling
One you have fulfilled the prerequisites:
```
git clone htpp://url/to/github.io
cd cv2xc-msg-tx
make build
```

This will create an executable called `transmitter` in `build/`

# Typical Usage
This project is in very early stages, so many of the parameters that the interface provides are ignored, and default hard-coded values are used instead. 

Some typical commands I've used to run this in the past are:
```
./build/transmitter -m abcd -a "clock_source=gpsdo,time_source=gpsdo"
```

```
./build/transmitter -m abcd -a "clock_source=external,time_source=internal"
```