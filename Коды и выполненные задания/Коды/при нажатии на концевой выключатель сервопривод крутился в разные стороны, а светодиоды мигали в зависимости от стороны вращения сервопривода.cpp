#include "dynamixel_sdk.h"
#include <iostream>
#include <unistd.h>

#define PORT_NAME "/dev/ttyS2"
#define SERVO_ID 1
#define LED_LEFT_ID 8
#define LED_RIGHT_ID 9
#define SWITCH_ID 23

struct BlinkState {
    bool left_on, right_on;
    bool left_state, right_state;
    unsigned long last_time;
};

BlinkState blink_state = {false, false, false, false, 0};

void updateBlinkState() {
    unsigned long current_time = clock() * 1000000 / CLOCKS_PER_SEC;
    if (current_time - blink_state.last_time >= 100000) {
        if (blink_state.left_on) blink_state.left_state = !blink_state.left_state;
        else blink_state.left_state = false;
        if (blink_state.right_on) blink_state.right_state = !blink_state.right_state;
        else blink_state.right_state = false;
        blink_state.last_time = current_time;
    }
}

void applyLEDStates(dynamixel::PacketHandler* packet, dynamixel::PortHandler* port) {
    uint8_t error = 0;
    
    uint8_t left_val = blink_state.left_state ? 200 : 0;
    if (packet->ping(port, LED_LEFT_ID, nullptr, &error) == COMM_SUCCESS)
        packet->write1ByteTxRx(port, LED_LEFT_ID, 26, left_val, &error);
    
    uint8_t right_val = blink_state.right_state ? 200 : 0;
    if (packet->ping(port, LED_RIGHT_ID, nullptr, &error) == COMM_SUCCESS)
        packet->write1ByteTxRx(port, LED_RIGHT_ID, 26, right_val, &error);
}

void setBlinkPattern(bool left, bool right) {
    blink_state.left_on = left;
    blink_state.right_on = right;
    blink_state.left_state = left;
    blink_state.right_state = right;
    blink_state.last_time = clock() * 1000000 / CLOCKS_PER_SEC;
}

int main() {
    std::cout << "=== MIXED PROTOCOL SYSTEM ===" << std::endl;
    
    bool servo_right = true;
    bool last_switch = false;
    blink_state.last_time = clock() * 1000000 / CLOCKS_PER_SEC;
    
    // Инициализация сервопривода
    dynamixel::PortHandler* servo_port = dynamixel::PortHandler::getPortHandler(PORT_NAME);
    dynamixel::PacketHandler* servo_packet = dynamixel::PacketHandler::getPacketHandler(2.0);
    
    if (servo_port->openPort() && servo_port->setBaudRate(9600)) {
        usleep(2000000);
        uint8_t error = 0;
        servo_packet->write1ByteTxRx(servo_port, SERVO_ID, 64, 1, &error);
        servo_packet->write1ByteTxRx(servo_port, SERVO_ID, 11, 1, &error);
        servo_packet->write4ByteTxRx(servo_port, SERVO_ID, 104, 200, &error);
        servo_port->closePort();
    }
    
    // Инициализация светодиодов
    setBlinkPattern(false, true);
    
    while (true) {
        updateBlinkState();
        
        unsigned long current_time = clock() * 1000000 / CLOCKS_PER_SEC;
        static unsigned long last_led_update = 0;
        if (current_time - last_led_update >= 20000) {
            dynamixel::PortHandler* update_port = dynamixel::PortHandler::getPortHandler(PORT_NAME);
            dynamixel::PacketHandler* update_packet = dynamixel::PacketHandler::getPacketHandler(1.0);
            if (update_port->openPort() && update_port->setBaudRate(57600)) {
                applyLEDStates(update_packet, update_port);
                update_port->closePort();
                last_led_update = current_time;
            }
        }
        
        // Проверка выключателя
        dynamixel::PortHandler* sw_port = dynamixel::PortHandler::getPortHandler(PORT_NAME);
        dynamixel::PacketHandler* sw_packet = dynamixel::PacketHandler::getPacketHandler(1.0);
        
        if (sw_port->openPort() && sw_port->setBaudRate(57600)) {
            usleep(100000);
            uint8_t state, error = 0;
            sw_packet->read1ByteTxRx(sw_port, SWITCH_ID, 27, &state, &error);
            sw_port->closePort();
            
            bool pressed = (state == 1);
            
            if (pressed && !last_switch) {
                servo_right = !servo_right;
                
                dynamixel::PortHandler* s_port = dynamixel::PortHandler::getPortHandler(PORT_NAME);
                dynamixel::PacketHandler* s_packet = dynamixel::PacketHandler::getPacketHandler(2.0);
                
                if (s_port->openPort() && s_port->setBaudRate(9600)) {
                    usleep(100000);
                    
                    if (servo_right) {
                        s_packet->write4ByteTxRx(s_port, SERVO_ID, 104, 200, &error);
                        setBlinkPattern(false, true);
                        std::cout << "RIGHT" << std::endl;
                    } else {
                        s_packet->write4ByteTxRx(s_port, SERVO_ID, 104, -200, &error);
                        setBlinkPattern(true, false);
                        std::cout << "LEFT" << std::endl;
                    }
                    
                    s_port->closePort();
                }
            }
            
            last_switch = pressed;
        }
        
        usleep(5000);
    }
    
    return 0;
}

