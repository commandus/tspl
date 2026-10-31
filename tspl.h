#ifndef TSPL_TSPL_H
#define TSPL_TSPL_H

#include <string>
#include <vector>
#include <cstdint>

/**
 * CTX is an "printer system context"
 * In the Linux CTX is USB library context getting in the library initialization:
 *   if (libusb_init(&ctx) < 0) return -1;
 *   ...
 *   libusb_exit(ctx);  // free context
 * In the Windows context is 0 and is not used.
 */
#if defined(_MSC_VER) || defined(__MINGW32__)
#define TARGET_WINDOWS  1
#include <windows.h>
#define DEVICE_HANDLE HANDLE
#define CTX int
#else
#define TARGET_LINUX  1
#include <libusb-1.0/libusb.h>
#define DEVICE_HANDLE libusb_device_handle*
#define CTX libusb_context
#endif

/***
 * UsbPrinterName identifies printer and used in @see findUsbPrinters().
 * In the Windows, UsbPrinterName contains Windows device path like:
 *   \\?\USB#VID_0483&PID_5720#USB001#{28D78FAD-5A12-11D1-AE5B-0000F803A8C2}
 * where
 *  28D78FAD-5A12-11D1-AE5B-0000F803A8C2 magic GUID of the USB device driver
 * In the Linux, UsbPrinterName contains USB vendor and product identifiers not in the
 * path but separately.
 *
 */
class UsbPrinterName {
public:
    ///< USB vendor identifier (in Windows not used to open device)
    uint16_t vid;
    ///< USB product identifier (in Windows not used to open device)
    uint16_t pid;
    ///< Printer friendly name
    std::string name;
    ///< Windows raw device path
    std::string path;

    UsbPrinterName();
    UsbPrinterName(uint16_t aVid, uint16_t aPid);
    UsbPrinterName(uint16_t aVid, uint16_t aPid, const std::string &path);
    UsbPrinterName(uint16_t aVid, uint16_t aPid, const std::string &path, const std::string &name);
    UsbPrinterName(const UsbPrinterName &name);
};

/**
 * Search attached USB printers in the system
 * @param ctx libusb context pointer in Linux, 0 in Windows
 * @param retVal returns list of UsbPrinterName
 * @return 0- success
 * @see UsbPrinterName
 */
int findUsbPrinters(
        CTX *ctx,
        std::vector<UsbPrinterName> &retVal
);

/**
 * Modern TSPL printer can return DPI, width and height,
 * model name, serial number ans software version on request, older printer does not.
 * Anyway to print we need width and height at least in some cases.
 */
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

/**
 * TSPL usb printer has
 * - printer name
 * - printer properties
 */
class UsbPrinter {
public:
    CTX *ctx;
    UsbPrinterName name;
    UsbPrinterProperties properties;
    DEVICE_HANDLE deviceHandle;
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

bool PrintTSPL2(
    const std::string& usbDevicePath,
    const std::string& tsplCommands
);

bool PrintTSPL(
    const std::string& printerName,
    const std::string& tsplCommands
);

#endif
