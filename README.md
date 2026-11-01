

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

## Device list

| Bus description | MC Brand name      | USB Vendor id | USB Product id | DPI | Width, mm | Height, mm | Width, px | Height, px |
|----------------------|--------------------|---------------|----------------|-----|-----------|------------|-----------|------------|
| P1                   | STMicroelectronics | 0483          | 5720           | 203 | 56 (58)   | 60 *       | 448       | 480        |
|                      | STMicroelectronics | 1155          |                |     |           |            |           |            |
|                      | TSC                | 1208          |                |     |           |            |           |            |
|                      | Zebra              | 0A5C          |                |     |           |            |           |            |
|                      | Zebra              | 0A90          |                |     |           |            |           |            |
|                      | Xprinter           | 0451          |                |     |           |            |           |            |
|                      | Xprinter           | 1343          |                |     |           |            |           |            |
|                      | Xprinter           | 1203          |                |     |           |            |           |            |
|                      | Xprinter           | 0451          |                |     |           |            |           |            |

*  Width is the maximum value, while height depends on the actual height of the label.

1 inch = 25.4mm

## References

(How to send data to usb printers under Windows)[https://web.archive.org/web/20130126085547/http://peter.skarpetis.com/wiki/index.php/Usbprint.sys]
()[https://web.archive.org/web/20130126085547/http://blog.peter.skarpetis.com/wp-content/images/usbprint.c]
