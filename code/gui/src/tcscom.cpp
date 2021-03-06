#include "tcscom.h"

tcscom::tcscom(ftdi *device, QObject *parent) :
    QObject(parent)
{
    this->ftdi_device = device;
    pb_istream = pb_istream_from_buffer(pb_ibuffer, sizeof(pb_ibuffer));
    pb_ostream = pb_ostream_from_buffer(pb_obuffer, sizeof(pb_obuffer));
}

tcscom::~tcscom()
{

}

bool tcscom::setSettings(const settings_t* settings)
{
    quint8 cmd[4] = {SER_MAGIC1, SER_MAGIC2, CMD_SAVE_SETTINGS, 0};
    quint8 cs;

    pb_encode(&pb_ostream, settings_t_fields, &settings);
    quint8 len = pb_ostream.bytes_written;
    cmd[3] = len+4;
    cs = doChecksum(cmd, sizeof(cmd));
    cs += doChecksum(pb_obuffer, len);


    this->ftdi_device->write(cmd, sizeof(cmd));
    this->ftdi_device->write(pb_obuffer, len);
    this->ftdi_device->write(&cs);

    return false;
}

bool tcscom::getSettings(settings_t* settings)
{
    settings_t settings_tmp;
    quint8 cmd[5] = {SER_MAGIC1, SER_MAGIC2, CMD_SEND_SETTINGS, sizeof(cmd), 0};
    quint8 info[4];
    quint8 cs1, cs2;

    cmd[4] = doChecksum(cmd, sizeof(cmd));
    this->ftdi_device->write(cmd, sizeof(cmd));

    msleep(100);
    this->ftdi_device->read(info, sizeof(info));
    quint8 len = info[3]-4;

    this->ftdi_device->read(pb_ibuffer, len);
    this->ftdi_device->read(&cs1);

    cs2 = doChecksum(cmd, sizeof(cmd));
    cs2 += doChecksum(pb_ibuffer, len);

    if (cs1 != cs2)
    {
        PrintErrorDetails("Checksum failed!");
        return true;
    }

    pb_decode(&pb_istream, settings_t_fields, &settings_tmp);

    *settings = settings_tmp;

    return false;
}

bool tcscom::getInfo(status_t* status)
{
    status_t status_tmp;
    quint8 cmd[5] = {SER_MAGIC1, SER_MAGIC2, CMD_SEND_SETTINGS, sizeof(cmd), 0};
    quint8 info[4];
    quint8 cs;

    cmd[4] = doChecksum(cmd, sizeof(cmd));
    this->ftdi_device->write(cmd, sizeof(cmd));

    msleep(100);
    this->ftdi_device->read(info, sizeof(info));
    quint8 len = info[3]-4;

    this->ftdi_device->read(pb_ibuffer, len);
    this->ftdi_device->read(&cs);

    pb_decode(&pb_istream, status_t_fields, &status_tmp);

    *status = status_tmp;

    return false;
}

bool tcscom::getDiag(sensors_t* sensors)
{
    sensors_t sensors_tmp;
    quint8 cmd[5] = {SER_MAGIC1, SER_MAGIC2, CMD_SEND_SETTINGS, sizeof(cmd), 0};
    quint8 info[4];
    quint8 cs;

    cmd[4] = doChecksum(cmd, sizeof(cmd));
    this->ftdi_device->write(cmd, sizeof(cmd));

    msleep(100);
    this->ftdi_device->read(info, sizeof(info));
    quint8 len = info[3]-4;

    this->ftdi_device->read(pb_ibuffer, len);
    this->ftdi_device->read(&cs);

    pb_decode(&pb_istream, sensors_t_fields, &sensors_tmp);

    *sensors = sensors_tmp;

    return false;
}

quint8 tcscom::doChecksum(quint8 *buf, quint8 len)
{
    quint8 sum = 0;

    while (len--)
        sum += *buf++;

    return sum;
}
