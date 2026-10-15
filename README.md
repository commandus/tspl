

## Linux

```shell
sudo apt-get install libusb-1.0-0-dev
```
`
```shell
...
sudo dmesg
usb 4-1.1: new full-speed USB deviceHandle number 4 using ehci-pci
usb 4-1.1: New USB deviceHandle found, idVendor=0483, idProduct=5720, bcdDevice= 2.00
usb 4-1.1: New USB deviceHandle strings: Mfr=1, Product=2, SerialNumber=3
usb 4-1.1: Product: P1
usb 4-1.1: Manufacturer: GEZHI
usb 4-1.1: SerialNumber: USB001
usblp 4-1.1:1.0: usblp1: USB Bidirectional printer dev 4 if 0 alt 0 proto 2 vid 0x0483 pid 0x5720
usbcore: registered new interface driver usblp
```

```shell
lsusb
...
Bus 004 Device 004: ID 0483:5720 STMicroelectronics Mass Storage Device
```

```shell
sudo vi /etc/udev/rules.d/99-printer-tspl.rules
SUBSYSTEM=="usb", ATTR{idVendor}=="0483", ATTR{idProduct}=="5720", MODE="0666", GROUP="plugdev"

sudo udevadm control --reload-rules
sudo udevadm trigger
```
## References

(How to send data to usb printers under Windows)[https://web.archive.org/web/20130126085547/http://peter.skarpetis.com/wiki/index.php/Usbprint.sys]
()[https://web.archive.org/web/20130126085547/http://blog.peter.skarpetis.com/wp-content/images/usbprint.c]
