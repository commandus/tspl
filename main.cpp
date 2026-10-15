#include <iostream>
#include <iomanip>
#include "tspl.h"

int main() {
    // device interface, which is used in Windows for USB printer classes
    std::string usbDevicePath = "\\\\?\\USB#VID_0483&PID_5720#USB001#{28D78FAD-5A12-11D1-AE5B-0000F803A8C2}";
    std::string tspl =
            "SIZE 40 mm, 30 mm\r\n"
            "GAP 2 mm, 0\r\n"
            "CLS\r\n"
            "TEXT 50,50,\"ROMAN.TTF\",0,1,1,\"C++ TSPL PRINT\"\r\n"
            "PRINT 1,1\r\n";


    std::vector<UsbPrinterName> printerNames;

    if (findUsbPrinters(printerNames) > 0) {
        for (auto printerName = printerNames.begin(); printerName != printerNames.end(); printerName++) {
            std::cout << printerName->name << "\t" << printerName->path << "\t" << std::hex
                      << std::setfill('0') << std::setw(4) << printerName->vid << ":" << printerName->pid;
            std::cout << std::endl;
            UsbPrinter printer(*printerName);
            printer.readProperties();
            std::cout << "\t" << printer.properties.model
                << "\t S/N " << printer.properties.serialNumber
                << "\t v. " << printer.properties.versionNumber
                << "\t " << (int) printer.properties.dpi << "dpi"
                << "\t " << (int) printer.properties.width << "x" << (int) printer.properties.height
                << std::endl;
        }
        // PrintTSPL2(usbDevicePath, tspl);
    }
    return 0;
}
