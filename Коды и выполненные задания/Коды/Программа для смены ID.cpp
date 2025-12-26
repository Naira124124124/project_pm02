#include "dynamixel_sdk.h"
#include <iostream>
#include <unistd.h>

#define PORT_NAME "/dev/ttyS2"
#define OLD_ID 9
#define NEW_ID 8
#define BAUDRATE 57600
#define PROTOCOL 1.0

int main() {
    std::cout << "=== CHANGE LED ID ===" << std::endl;
    std::cout << "Changing ID from " << OLD_ID << " to " << NEW_ID << std::endl;
    std::cout << "Protocol: " << PROTOCOL << ", Baud rate: " << BAUDRATE << "\n" << std::endl;
    
    dynamixel::PortHandler* port = dynamixel::PortHandler::getPortHandler(PORT_NAME);
    dynamixel::PacketHandler* packet = dynamixel::PacketHandler::getPacketHandler(PROTOCOL);
    
    if (!port->openPort() || !port->setBaudRate(BAUDRATE)) {
        std::cout << "Port error!" << std::endl;
        return -1;
    }
    
    usleep(1000000);
    
    uint8_t error = 0;
    uint16_t model;
    
    std::cout << "1. Checking device with old ID " << OLD_ID << "..." << std::endl;
    if (packet->ping(port, OLD_ID, &model, &error) == COMM_SUCCESS) {
        std::cout << "Device found (Model: " << model << ")" << std::endl;
    } else {
        std::cout << "Device not found!" << std::endl;
        port->closePort();
        return -1;
    }
    
    std::cout << "\nWARNING: This will change ID permanently!" << std::endl;
    std::cout << "   Press Enter to continue...";
    std::cin.ignore();
    
    std::cout << "\n2. Changing ID..." << std::endl;
    if (packet->write1ByteTxRx(port, OLD_ID, 3, NEW_ID, &error) == COMM_SUCCESS) {
        std::cout << "ID change command sent" << std::endl;
    } else {
        std::cout << "Failed!" << std::endl;
        port->closePort();
        return -1;
    }
    
    sleep(3);
    
    std::cout << "\n3. Verifying..." << std::endl;
    std::cout << "   Old ID " << OLD_ID << "... ";
    if (packet->ping(port, OLD_ID, &model, &error) == COMM_SUCCESS) {
        std::cout << "STILL RESPONDS!" << std::endl;
    } else {
        std::cout << "no response" << std::endl;
    }
    
    std::cout << "   New ID " << NEW_ID << "... ";
    if (packet->ping(port, NEW_ID, &model, &error) == COMM_SUCCESS) {
        std::cout << "RESPONDS!" << std::endl;
        std::cout << "   Model: " << model << std::endl;
    } else {
        std::cout << "no response" << std::endl;
    }
    
    std::cout << "\n4. Testing LED..." << std::endl;
    std::cout << "   Turning ON..." << std::endl;
    packet->write1ByteTxRx(port, NEW_ID, 26, 200, &error);
    sleep(2);
    
    std::cout << "   Turning OFF..." << std::endl;
    packet->write1ByteTxRx(port, NEW_ID, 26, 0, &error);
    sleep(1);
    
    std::cout << "   Blinking..." << std::endl;
    for (int i = 0; i < 3; i++) {
        packet->write1ByteTxRx(port, NEW_ID, 26, 150, &error);
        usleep(200000);
        packet->write1ByteTxRx(port, NEW_ID, 26, 0, &error);
        usleep(200000);
    }
    
    port->closePort();
    
    std::cout << "\nID change complete!" << std::endl;
    std::cout << "   Old: " << OLD_ID << " → New: " << NEW_ID << std::endl;
    
    return 0;
}
