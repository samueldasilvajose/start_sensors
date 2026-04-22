# start_sensors


```
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
while true; do cansend vcan0 221#01; sleep 1.0; done //mensagem de status
```
