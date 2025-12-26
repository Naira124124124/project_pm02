#include "dynamixel_sdk.h"
#include <unistd.h>

int main() {
    dynamixel::PortHandler* port = dynamixel::PortHandler::getPortHandler("/dev/ttyS2");
    if (!port->openPort()) return -1;
    
    dynamixel::PacketHandler* p1 = dynamixel::PacketHandler::getPacketHandler(1.0);
    dynamixel::PacketHandler* p2 = dynamixel::PacketHandler::getPacketHandler(2.0);
    
    port->setBaudRate(9600);
    p2->write1ByteTxRx(port, 1, 64, 1, nullptr);
    p2->write1ByteTxRx(port, 1, 11, 1, nullptr);
    
    bool dir = true;
    bool switch_state = false;
    
    while (true) {
        port->setBaudRate(57600);
        usleep(100000);
        
        uint8_t sw;
        if (p1->read1ByteTxRx(port, 23, 27, &sw, nullptr) == COMM_SUCCESS) {
            if (sw == 1 && !switch_state) {
                dir = !dir;
                port->setBaudRate(9600);
                usleep(100000);
                
                if (dir) p2->write4ByteTxRx(port, 1, 104, 250, nullptr);
                else p2->write4ByteTxRx(port, 1, 104, -250, nullptr);
            }
            switch_state = (sw == 1);
        }
        usleep(100000);
    }
    
    return 0;
}
