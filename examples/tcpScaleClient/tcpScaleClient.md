## tcpScaleClient example
This example demonstrates the transfer of multiple data types (strings, integers, and floats) between 2 C++ programs on the network. A TCP socket server is hosted in the server program and this program is the client. A basic command word interface is set up and used. The code in this example can be expanded upon to allow for sensor data to be sent to another system on the network.

### additional info ###
Useful package for i2c scanning:
```bash
sudo apt install i2c-tools -y
i2cdetect -y 1
# enable the second i2c port on a raspberry pi 
# by adding "dtparam=i2c_vc=on" to /boot/config.txt
i2cdetect -y 2
```

## Build
use the following commands to build this example:
(run commands from the tcpScaleClient directory)
```bash
make
make all
```

## Run
1. Start the server program (see MissionControl Repo)

2. Start the executable for this example using:
```bash
make run
# or
make ARGS="<server-ip> <server-port>" run
# or
make run ARGS="<server-ip> <server-port>"
# ex:
make run ARGS="192.168.1.197 13056"
# read Makefile to see available commands
```




