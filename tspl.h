#ifndef TSPL_TSPL_H
#define TSPL_TSPL_H

#include <string>
#include <vector>
#include <cstdint>

#if defined(_MSC_VER) || defined(__MINGW32__)
#define TARGET_WINDOWS  1
#include <windows.h>
#define DEVICE_HANDLE HANDLE
#define CTX int
#else
#define TARGET_LINUX  1
#include <libusb-1.0/libusb.h>
#define DEVICE_HANDLE libusb_device_handle
#define CTX libusb_context
#endif

class UsbPrinterName {
public:
    uint16_t vid;
    uint16_t pid;
    std::string name;
    std::string path;

    UsbPrinterName();
    UsbPrinterName(uint16_t aVid, uint16_t aPid);
    UsbPrinterName(uint16_t aVid, uint16_t aPid, const std::string &path);
    UsbPrinterName(uint16_t aVid, uint16_t aPid, const std::string &path, const std::string &name);
    UsbPrinterName(const UsbPrinterName &name);
};

class UsbPrinterProperties {
public:
    std::string model;
    std::string serialNumber;
    std::string versionNumber;

    uint16_t dpi;
    uint16_t width;
    uint16_t height;

    UsbPrinterProperties();
    UsbPrinterProperties(const UsbPrinterProperties& value);
};

class UsbPrinter {
public:
    CTX *ctx;
    UsbPrinterName name;
    UsbPrinterProperties properties;
    DEVICE_HANDLE *deviceHandle;
#ifdef TARGET_LINUX
    int interfaceNumber;
#endif
    UsbPrinter(CTX *ctx, const UsbPrinterName& name);
    UsbPrinter(const UsbPrinter& value);
    virtual ~UsbPrinter();
    int open();
    void close();
    int write(const void* buffer, size_t len);
    int write(const std::string &value);
    void delay();
    int read(std::string& retVal);
    int query(std::string& retVal, const std::string& request);
    int readProperties();
};

int findUsbPrinters(
    CTX *ctx,
    std::vector<UsbPrinterName> &retVal
);

bool PrintTSPL2(
    const std::string& usbDevicePath,
    const std::string& tsplCommands
);

bool PrintTSPL(
    const std::string& printerName,
    const std::string& tsplCommands
);

#endif
