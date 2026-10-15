#ifndef TSPL_TSPL_H
#define TSPL_TSPL_H

#include <string>
#include <vector>

#if defined(_MSC_VER) || defined(__MINGW32__)
#define TARGET_WINDOWS  1
#include <windows.h>
#define DEVICE_DESCRIPTOR HANDLE
#else
#define TARGET_LINUX  1
#define DEVICE_DESCRIPTOR int
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
    UsbPrinterName name;
    UsbPrinterProperties properties;
    DEVICE_DESCRIPTOR device;

    UsbPrinter(const UsbPrinterName& name);
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
