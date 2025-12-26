#include "dynamixel_sdk.h"
#include <unistd.h>

int main() {
    dynamixel::PortHandler* port = dynamixel::PortHandler::getPortHandler("/dev/ttyS2");
    dynamixel::PacketHandler* packet = dynamixel::PacketHandler::getPacketHandler(1.0);
    
    if (!port->openPort() || !port->setBaudRate(57600)) return -1;
    
    uint8_t error = 0;
    
    while (true) {
        packet->write1ByteTxRx(port, 21, 27, 255, &error);
        packet->write1ByteTxRx(port, 21, 28, 0, &error);
        packet->write1ByteTxRx(port, 21, 29, 0, &error);
        sleep(2);
        
        packet->write1ByteTxRx(port, 21, 27, 0, &error);
        packet->write1ByteTxRx(port, 21, 28, 255, &error);
        packet->write1ByteTxRx(port, 21, 29, 0, &error);
        sleep(2);
        
        packet->write1ByteTxRx(port, 21, 27, 0, &error);
        packet->write1ByteTxRx(port, 21, 28, 0, &error);
        packet->write1ByteTxRx(port, 21, 29, 255, &error);
        sleep(2);
        
        packet->write1ByteTxRx(port, 21, 27, 255, &error);
        packet->write1ByteTxRx(port, 21, 28, 255, &error);
        packet->write1ByteTxRx(port, 21, 29, 255, &error);
        sleep(2);
        
        packet->write1ByteTxRx(port, 21, 27, 0, &error);
        packet->write1ByteTxRx(port, 21, 28, 0, &error);
        packet->write1ByteTxRx(port, 21, 29, 0, &error);
        sleep(2);
    }
    
    port->closePort();
    return 0;
}
