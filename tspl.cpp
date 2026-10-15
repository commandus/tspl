#include "tspl.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <csignal>

#ifdef TARGET_WINDOWS
#include <regex>
#include <setupapi.h>
#include <initguid.h>
DEFINE_GUID(GUID_DEVINTERFACE_USBPRINT, 0x28d78fad, 0x5a12, 0x11D1, 0xae, 0x5b, 0x00, 0x00, 0xf8, 0x03, 0xa8, 0xc2);
#endif

enum TSPL_COMMAND {
    CMD_GETSETTING = 0,
    CMD_SYSTEM = 1,
    CMD_INFORMATION = 2,
    CMD_MODEL = 3,
    CMD_DPI = 4,
    CMD_SERIAL = 5,
    CMD_VERSION = 6
};

const char* TSPL_COMMAND_TEXT[] {
    "GETSETTING$",
    "SYSTEM",
    "INFORMATION",
    "MODEL",
    "DPI",
    "SERIAL",
    "VERSION"
};

std::string cmdString(TSPL_COMMAND cmd) {
    std::stringstream ss;
    switch (cmd) {
        case CMD_MODEL:
        case CMD_DPI:
        case CMD_SERIAL:
        case CMD_VERSION:
            ss << TSPL_COMMAND_TEXT[CMD_GETSETTING] << "(\""
                << TSPL_COMMAND_TEXT[CMD_SYSTEM] << "\",\""
                << TSPL_COMMAND_TEXT[CMD_INFORMATION] << "\",\""
                << TSPL_COMMAND_TEXT[cmd] << "\")";
            break;
    }
    ss << "\r\n";
    return ss.str();
}

#ifdef TARGET_LINUX
int findUsbPrinters(
    CTX *ctx,
    std::vector<UsbPrinterName> &retVal
)
{
    retVal.emplace_back(0x0483, 0x5720,"", "P1");
    return 1;
}
#endif

#ifdef TARGET_WINDOWS
int findUsbPrinters(
    CTX *ctx,
    std::vector<UsbPrinterName> &retVal
)
{
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(&GUID_DEVINTERFACE_USBPRINT, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return -1;

    SP_DEVICE_INTERFACE_DATA interfaceData;
    interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    int c = 0;
    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &GUID_DEVINTERFACE_USBPRINT, i, &interfaceData); ++i) {
        // Get buffer size
        DWORD detailSize = 0;
        SetupDiGetDeviceInterfaceDetailA(hDevInfo, &interfaceData, nullptr, 0, &detailSize, NULL);

        auto detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_A) malloc (detailSize);
        detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

        // Get path
        if (SetupDiGetDeviceInterfaceDetailA(hDevInfo, &interfaceData, detailData, detailSize, nullptr, nullptr)) {
            std::string path(detailData->DevicePath);

            std::string pathUpper = path;
            std::transform(pathUpper.begin(), pathUpper.end(), pathUpper.begin(), ::toupper);

            // Check path must looks like:
            // \\?\USB#VID_0483&PID_5720#USB001#{28D78FAD-5A12-11D1-AE5B-0000F803A8C2}
            //
            std::regex reg("VID_([0-9A-Fa-f]{4})&PID_([0-9A-Fa-f]{4})#(.*)#");
            std::smatch matches;

            if (std::regex_search(pathUpper, matches, reg)) {
                SP_DEVINFO_DATA devInfoData;
                devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                SetupDiGetDeviceInterfaceDetailA(hDevInfo, &interfaceData, NULL, 0, &detailSize, &devInfoData);

                DWORD propertyRegDataType = 0;
                char friendlyName[256] = {0};
                if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME,
                    &propertyRegDataType, (PBYTE) friendlyName, sizeof(friendlyName), NULL)) {
                }

                retVal.emplace_back(
                std::stoul(matches[1].str(), nullptr, 16),
                std::stoul(matches[2].str(), nullptr, 16),
                path, friendlyName
                );
                c++;
            }
        }
        free(detailData);
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return c;
}

bool PrintTSPL2(const std::string& usbDevicePath, const std::string& tsplCommands) {
    // Открываем USB-устройство как обычный файл для записи
    HANDLE hUsb = CreateFileA(
            (LPCSTR) usbDevicePath.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
    );

    if (hUsb == INVALID_HANDLE_VALUE) {
        std::cerr << "Can not open USB port " << usbDevicePath << " . Error: " << GetLastError() << std::endl;
        return false;
    }

    DWORD bytesWritten;

    // Просто пишем текст напрямую в USB-кабель
    if (WriteFile(hUsb, tsplCommands.c_str(), tsplCommands.length(), &bytesWritten, nullptr)) {
        std::cout << "Wrote successfully" << std::endl;
    }

    CloseHandle(hUsb);
    return true;
}

bool PrintTSPL(const std::string& printerName, const std::string& tsplCommands) {
    HANDLE hPrinter = NULL;
    DOC_INFO_1A docInfo;
    DWORD bytesWritten = 0;

    // 1. Открываем принтер по его имени в Windows (например, "P1")
    if (!OpenPrinterA((LPSTR)printerName.c_str(), &hPrinter, NULL)) {
        std::cerr << "Can not open printer. Error: " << GetLastError() << std::endl;
        return false;
    }

    // 2. Заполняем структуру информации о документе
    // Режим "RAW" обязателен — он говорит Windows передать байты в USB без изменений
    docInfo.pDocName = (LPSTR)"TSPL Label Job";
    docInfo.pOutputFile = NULL;
    docInfo.pDatatype = (LPSTR)"RAW";

    // 3. Начинаем документ и страницу печати
    if (StartDocPrinterA(hPrinter, 1, (LPBYTE)&docInfo) == 0) {
        std::cerr << "Не удалось начать документ. Ошибка: " << GetLastError() << std::endl;
        ClosePrinter(hPrinter);
        return false;
    }

    if (!StartPagePrinter(hPrinter)) {
        std::cerr << "Не удалось начать страницу. Ошибка: " << GetLastError() << std::endl;
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);
        return false;
    }

    // 4. Отправляем TSPL команды напрямую в USB-порт принтера
    BOOL success = WritePrinter(
            hPrinter,
            (LPVOID)tsplCommands.c_str(),
            (DWORD)tsplCommands.length(),
            &bytesWritten
    );

    // 5. Чистим за собой ресурсы
    EndPagePrinter(hPrinter);
    EndDocPrinter(hPrinter);
    ClosePrinter(hPrinter);

    if (!success || bytesWritten != tsplCommands.length()) {
        std::cerr << "Ошибка при записи данных в принтер." << std::endl;
        return false;
    }

    return true;
}
#endif

UsbPrinterName::UsbPrinterName()
    : vid(0), pid(0)
{

}

UsbPrinterName::UsbPrinterName(
    uint16_t aVid,
    uint16_t aPid
)
    : vid(aVid), pid(aPid)
{

}

UsbPrinterName::UsbPrinterName(
    uint16_t aVid,
    uint16_t aPid,
    const std::string &aPath
)
    : vid(aVid), pid(aPid), path(aPath)
{

}

UsbPrinterName::UsbPrinterName(
    uint16_t aVid,
    uint16_t aPid,
    const std::string &aPath,
    const std::string &aName
)
    : vid(aVid), pid(aPid), path(aPath), name(aName)
{

}

UsbPrinterName::UsbPrinterName(
    const UsbPrinterName &name
)
    : vid(name.vid), pid(name.pid), path(name.path), name(name.name)
{

}

UsbPrinter::UsbPrinter(
    CTX *aCtx,
    const UsbPrinterName &aName
)
    : ctx(aCtx), name(aName)
{
    open();
}

int UsbPrinter::open()
{
#ifdef TARGET_WINDOWS
    deviceHandle = CreateFileA(
            name.path.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
    );
    if (deviceHandle == INVALID_HANDLE_VALUE)
        return -1;
#endif
#ifdef TARGET_LINUX
    deviceHandle = libusb_open_device_with_vid_pid(ctx, name.vid, name.pid);
    if (!deviceHandle)
        return -1;
    interfaceNumber = 0;
    if (libusb_kernel_driver_active(deviceHandle, interfaceNumber) == 1) {
        std::cout << "Драйвер ядра активен. Пытаемся отключить..." << std::endl;
        if (libusb_detach_kernel_driver(deviceHandle, interfaceNumber) == 0) {
            std::cout << "Системный драйвер успешно отключен." << std::endl;
        } else {
            std::cerr << "Ошибка: Не удалось отключить драйвер ядра." << std::endl;
            return -1;
        }
    }

    int r = libusb_claim_interface(deviceHandle, interfaceNumber);
    if (r < 0) {
        std::cerr << "Ошибка: Не удалось заявить права на интерфейс (" << r << ")." << std::endl;
        return -1;
    }

#endif
    readProperties();
    return 0;
}

void UsbPrinter::close() {
#ifdef TARGET_WINDOWS
    CloseHandle(deviceHandle);
    deviceHandle = nullptr;
#endif
#ifdef TARGET_LINUX
    libusb_release_interface(deviceHandle, interfaceNumber);
    libusb_attach_kernel_driver(deviceHandle, interfaceNumber);
    libusb_close(deviceHandle);
#endif
}

int UsbPrinter::query(
    std::string& retVal,
    const std::string& request
) {
    int r = write(request);
    if (r)
        return r;
    delay();
    return read(retVal);
}

int UsbPrinter::readProperties() {

    std::string status;
    // "\x1b!?" ""
    int r = query(status, "{STATUS}\r\n");
    if (r)
        return r;

    r = query(status, "~!T\r\n");
    if (r)
        return r;

    r = query(properties.model, cmdString(CMD_MODEL));
    if (r)
        return r;
    std::string s;
    r = query(s, cmdString(CMD_DPI));
    if (r)
        return r;
    properties.dpi = strtol(s.c_str(), nullptr, 10);
    r = query(properties.serialNumber, cmdString(CMD_SERIAL));
    if (r)
        return r;
    r = query(properties.versionNumber, cmdString(CMD_VERSION));
    return r;
}

int UsbPrinter::write(
    const void* buffer,
    size_t len
)
{
#ifdef TARGET_WINDOWS
    DWORD bytesWritten;
    if (!WriteFile(deviceHandle, buffer, (DWORD) len, &bytesWritten, nullptr))
        return -1;
#endif
#ifdef TARGET_LINUX
    int actual_length = 0;
    unsigned int timeout_ms = 5000;
    uint8_t endpoint_out = 1;
    int r = libusb_bulk_transfer(
            deviceHandle,
            endpoint_out,
            (unsigned char *) buffer,
            len, &actual_length, timeout_ms
    );
#endif
    return 0;
}

int UsbPrinter::write(
    const std::string &value
)
{
    return write(value.c_str(), value.size());
}

void UsbPrinter::delay() {
#ifdef TARGET_WINDOWS
    Sleep(200);
#endif
#ifdef TARGET_LINUX
    sleep(1);
#endif
}

int UsbPrinter::read(std::string &retVal) {
#ifdef TARGET_WINDOWS
    char buffer[256];
    DWORD bytesRead = 0;
    if (!ReadFile(deviceHandle, buffer, (DWORD) (sizeof buffer), &bytesRead, nullptr))
        return -1;
    retVal = std::string(buffer, bytesRead);
#endif
    return 0;
}

UsbPrinter::UsbPrinter(
    const UsbPrinter &value
)
    : ctx(value.ctx), name(value.name), properties(value.properties), deviceHandle(value.deviceHandle)
{
    open();
}

UsbPrinter::~UsbPrinter() {
    close();
}

UsbPrinterProperties::UsbPrinterProperties()
    : dpi(0), width(0), height(0)
{

}

UsbPrinterProperties::UsbPrinterProperties(
    const UsbPrinterProperties &value
)
    : model(value.model), serialNumber(value.serialNumber), versionNumber(value.versionNumber),
    dpi(value.dpi), width(value.width), height(value.height)
{

}
