#ifndef SERIAL_TRANSPORT_H
#define SERIAL_TRANSPORT_H

#include "TransportBase.h"

namespace transport
{

// Serial port transport implementation
class SerialTransport : public TransportBase
{
public:
    explicit SerialTransport(const SerialConfig& config);
    virtual ~SerialTransport() override;

    // ITransport interface implementation
    bool Open() override;
    void Close() override;
    int Send(const void* data, size_t size) override;
    int Receive(void* buffer, size_t size) override;
    TransportType GetType() const override;

private:
    // Configure serial port parameters
    bool ConfigurePort();

    // Convert baud rate integer to termios constant
    int GetBaudRateConstant(int baudrate);

    SerialConfig m_config;
};

} // namespace transport

#endif // SERIAL_TRANSPORT_H
