#include "ftdi.h"

ftdi::ftdi(QObject *parent) :
    QObject(parent)
{
    iNumDevs = 0;
    pcBufLD[0] = cBufLD;

    ftEepromHeader.deviceType = FT_DEVICE_X_SERIES;
    eepromDATA.common = ftEepromHeader;
    eepromDATA.common.deviceType = FT_DEVICE_X_SERIES;
}

bool ftdi::connect(void)
{

    this->ftStatus = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);

    if(ftStatus != FT_OK) {
        qWarning("Error: FT_ListDevices(%d)\n", (int)ftStatus);
        return true;
    }

    if((ftStatus = FT_OpenEx(cBufLD, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle)) != FT_OK){
        /*
            This can fail if the ftdi_sio driver is loaded
            use lsmod to check this and rmmod ftdi_sio to remove
            also rmmod usbserial
        */
        qWarning("Error FT_OpenEx(%d), device %d\n", (int)ftStatus, 0);
        qWarning("Use lsmod to check if ftdi_sio (and usbserial) are present.\n");
        qWarning("If so, unload them using rmmod, as they conflict with ftd2xx.\n");
        return 1;
    }

    FT_SetTimeouts(ftHandle, 3000, 3000);	// 3 second read timeout

    if((ftStatus = FT_SetBaudRate(ftHandle, 115200)) != FT_OK) {
        qWarning("Error FT_SetBaudRate(%d), cBufLD[i] = %s\n", (int)ftStatus, cBufLD);
        return 1;
    }

    return this->resetBootloader();
}

bool ftdi::disconnect()
{
    this->setCBUS(0);

    return FT_Close(ftHandle) == FT_OK;
}

bool ftdi::write(char * buf, unsigned int len)
{
    unsigned int dwBytesWritten;
    ftStatus = FT_Write(ftHandle, buf, len, &dwBytesWritten);
    if (ftStatus != FT_OK) {
        qWarning("Error FT_Write(%d)\n", (int)ftStatus);
        return true;
    }
    return false;
}

bool ftdi::read(char * buf, unsigned int len)
{
    unsigned int dwBytesRead;
    ftStatus = FT_Read(ftHandle, buf, len, &dwBytesRead);
    if (ftStatus != FT_OK) {
        qWarning("Error FT_Read(%d)\n", (int)ftStatus);
        return true;
    }
    return false;
}

bool ftdi::readAll(char *buf, unsigned int max_len)
{
    unsigned int len = 0;

    ftStatus = FT_GetQueueStatus(ftHandle, &len);

    if(ftStatus != FT_OK) {
        qWarning("Error FT_GetQueueStatus(%d)\n", (int)ftStatus);
        return 1;
    }

    if (len > max_len)
    {
        len = max_len;
    }

    return this->read(buf, len);
}

bool ftdi::setCBUSMux(bool en)
{
    char Manufacturer[64];
    char ManufacturerId[64];
    char Description[64];
    char SerialNumber[64];

    ftStatus = FT_EEPROM_Read(ftHandle, &eepromDATA, sizeof(eepromDATA),
    Manufacturer, ManufacturerId, Description, SerialNumber);

    if (ftStatus != FT_OK)
    {
        printf("EEPROM_Read status not ok %d\n", ftStatus);
        return true;
    }

    /* Set drive current to 16ma */
    eepromDATA.ACDriveCurrent |= 48;

    /* CBUS0 is BOOT0 - CBUS3 is NRST */
    if (en)
    {
        eepromDATA.Cbus0 = FT_X_SERIES_CBUS_IOMODE;
        eepromDATA.Cbus3 = FT_X_SERIES_CBUS_IOMODE;
    }
    else /* Could also use FT_X_SERIES_CBUS_TRISTATE */
    {
        eepromDATA.Cbus0 = FT_X_SERIES_CBUS_DRIVE_0; /* BOOT0 to GND */
        eepromDATA.Cbus3 = FT_X_SERIES_CBUS_DRIVE_1; /* NRST to VDD */
    }

    ftStatus = FT_EEPROM_Program(ftHandle, &eepromDATA, sizeof(eepromDATA),
    Manufacturer, ManufacturerId, Description, SerialNumber);

    if (ftStatus != FT_OK)
    {
        printf("EEPROM_Program status not ok %d\n", ftStatus);
        return true;
    }

    return false;
}

bool ftdi::resetBootloader()
{
    int res;

    /* Reset low, BOOT0 high */
    res = setCBUS(CBUS2MASK(0,0,0,1));
    usleep(200000);

    /* Reset high, BOOT0 high */
    res = setCBUS(CBUS2MASK(1,0,0,1));
    usleep(200000);

    /* Back to input mode */
    res = setCBUS(0);

    return res;
}

bool ftdi::setCBUS(int mask)
{
    if(FT_SetBitMode(ftHandle, mask, FT_BITMODE_CBUS_BITBANG) != FT_OK) {
        qWarning("Failed to set CBUS bit mask: %d\n", mask);
        return 1;
    }

    return false;
}
