#include <unistd.h>
#include "dynamixel_sdk.h"
#include "fmt/format.h"
#define DEVICENAME "/dev/ttyS2"
#define PROTOCOL 2.0
#define BAUDRATE 57600
#define LED_ID 1
dynamixel::PortHandler *portHandler;
dynamixel::PacketHandler *packetHandler;

void scanDevices() {
    fmt::print("=== Scanning Dynamixel ===\n");
    uint8_t dxl_error;
    uint16_t model_number;
    for(int id = 0; id < 253; id++) {
        if(packetHandler->ping(portHandler, id, &model_number, &dxl_error) == COMM_SUCCESS)
            fmt::print("ID: {}, Model: {}\n", id, model_number);
    }
}
void checkRegisters() {
    uint8_t dxl_error, value;
    uint16_t value16;
    fmt::print("\n=== Checking registers ===\n");
    
    struct RegInfo { int addr; const char* name; bool is2b; };
    RegInfo regs[] = {{0, "Model", true}, {2, "Firmware", false}, {7, "ID", false},
                     {8, "Baud", false}, {11, "Mode", false}, {31, "Temp", false},
                     {64, "Torque", false}, {65, "LED", false}, {70, "Error", false}};
    
    for(int i = 0; i < sizeof(regs)/sizeof(regs[0]); i++) {
        if(regs[i].is2b) {
            if(packetHandler->read2ByteTxRx(portHandler, LED_ID, regs[i].addr, &value16, &dxl_error) == COMM_SUCCESS)
                fmt::print("{:10} [0x{:02X}]: {}\n", regs[i].name, regs[i].addr, value16);
        } else {
            if(packetHandler->read1ByteTxRx(portHandler, LED_ID, regs[i].addr, &value, &dxl_error) == COMM_SUCCESS)
                fmt::print("{:10} [0x{:02X}]: {}\n", regs[i].name, regs[i].addr, value);
        }
    }
}
void testLED() {
    uint8_t dxl_error;
    fmt::print("\n=== LED Test ===\n");
    
    packetHandler->write1ByteTxRx(portHandler, LED_ID, 65, 0, &dxl_error);
    sleep(1);
    packetHandler->write1ByteTxRx(portHandler, LED_ID, 65, 1, &dxl_error);
    sleep(1);
    for(int i = 0; i < 10; i++) {
        packetHandler->write1ByteTxRx(portHandler, LED_ID, 65, 1, &dxl_error);
        usleep(250000);
        packetHandler->write1ByteTxRx(portHandler, LED_ID, 65, 0, &dxl_error);
        usleep(250000);
        fmt::print(".");
    }
    fmt::print("\n");
}
void setPositionMode() {
    uint8_t dxl_error;
    packetHandler->write1ByteTxRx(portHandler, LED_ID, 64, 0, &dxl_error);
    sleep(1);
    packetHandler->write1ByteTxRx(portHandler, LED_ID, 11, 3, &dxl_error);
    packetHandler->write1ByteTxRx(portHandler, LED_ID, 64, 1, &dxl_error);
}
void testPosition() {
    uint8_t dxl_error;
    int positions[] = {1024, 2048, 3072, 2048};
    for(int i = 0; i < 4; i++) {
        packetHandler->write4ByteTxRx(portHandler, LED_ID, 116, positions[i], &dxl_error);
        sleep(2);
    }
}
int main() {
    portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);
    packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL);
    
    if(!portHandler->openPort() || !portHandler->setBaudRate(BAUDRATE)) {
        fmt::print("Port error\n");
        return 1;
    }
    scanDevices();
    uint8_t dxl_error;
    uint16_t model_number;
    if(packetHandler->ping(portHandler, LED_ID, &model_number, &dxl_error) != COMM_SUCCESS) {
        fmt::print("Ping failed\n");
        portHandler->closePort();
        return 1;
    }
    checkRegisters();
    testLED();
    setPositionMode();
    testPosition();
    
    packetHandler->write1ByteTxRx(portHandler, LED_ID, 64, 0, &dxl_error);
    portHandler->closePort();
    
    return 0;
}
