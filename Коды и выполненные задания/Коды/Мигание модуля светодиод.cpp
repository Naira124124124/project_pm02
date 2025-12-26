#include "dynamixel_sdk.h"
#include <unistd.h>
#include <vector>
#define PROTOCOL_VERSION 1.0
#define BAUDRATE 57600
#define PORT_NAME "/dev/ttyS2"

int main() {
    PortHandler* port = PortHandler::getPortHandler(PORT_NAME);
    PacketHandler* packet = PacketHandler::getPacketHandler(PROTOCOL_VERSION);
    
    if (!port->openPort() || !port->setBaudRate(BAUDRATE)) {
        return -1;
    }
    
    usleep(1000000);
    std::vector<int> found_ids;
    
    for (int id = 0; id <= 253; id++) {
        uint16_t model_number;
        uint8_t error = 0;
        
        if (packet->ping(port, id, &model_number, &error) == COMM_SUCCESS) {
            found_ids.push_back(id);
            
            if (id == 9) {
                uint8_t current_value;
                packet->read1ByteTxRx(port, id, 26, &current_value, &error);
                packet->write1ByteTxRx(port, id, 26, 200, &error);
                sleep(2);
                packet->write1ByteTxRx(port, id, 26, 0, &error);
                sleep(2);
            }
        }
        
        usleep(10000);
    }
    
    port->closePort();
    return 0;
}
