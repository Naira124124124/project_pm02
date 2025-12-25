#include "dynamixel_sdk.h"
#include <iostream>
#include <unistd.h>

using namespace dynamixel;

#define PORT_NAME "/dev/ttyS2"

// ID устройств
#define SERVO_ID 1        // Протокол 2.0, скорость 9600
#define LED_LEFT_ID 8     // Левый светодиод, протокол 1.0, скорость 57600
#define LED_RIGHT_ID 9    // Правый светодиод, протокол 1.0, скорость 57600  
#define SWITCH_ID 23      // Протокол 1.0, скорость 57600

// Регистры для протокола 1.0 (LED и выключатель)
#define LED_REGISTER_1 26      // Яркость светодиода (0-255)
#define SWITCH_REGISTER_1 27   // Состояние выключателя

// Регистры для протокола 2.0 (сервопривод)
#define TORQUE_ENABLE_2 64
#define OPERATING_MODE_2 11
#define GOAL_VELOCITY_2 104

// Функция для управления светодиодами
void setLEDs(PacketHandler* packet, PortHandler* port, bool left_on, bool right_on, uint8_t brightness = 200) {
    uint8_t error = 0;
    
    // Управляем левым светодиодом (ID 8)
    if (packet->ping(port, LED_LEFT_ID, nullptr, &error) == COMM_SUCCESS) {
        uint8_t left_value = left_on ? brightness : 0;
        packet->write1ByteTxRx(port, LED_LEFT_ID, LED_REGISTER_1, left_value, &error);
    }
    
    // Управляем правым светодиодом (ID 9)
    if (packet->ping(port, LED_RIGHT_ID, nullptr, &error) == COMM_SUCCESS) {
        uint8_t right_value = right_on ? brightness : 0;
        packet->write1ByteTxRx(port, LED_RIGHT_ID, LED_REGISTER_1, right_value, &error);
    }
}

int main() {
    std::cout << "=== MIXED PROTOCOL SYSTEM ===" << std::endl;
    std::cout << "Servo (ID=1): Protocol 2.0, 9600 baud" << std::endl;
    std::cout << "Left LED (ID=8): Protocol 1.0, 57600 baud" << std::endl;
    std::cout << "Right LED (ID=9): Protocol 1.0, 57600 baud" << std::endl;
    std::cout << "Switch (ID=23): Protocol 1.0, 57600 baud\n" << std::endl;
    
    bool servo_right = true; // true = вправо, false = влево
    bool last_switch = false;
    
    // 1. Инициализация сервопривода (протокол 2.0, 9600)
    std::cout << "1. Initializing servo..." << std::endl;
    
    PortHandler* servo_port = PortHandler::getPortHandler(PORT_NAME);
    PacketHandler* servo_packet = PacketHandler::getPacketHandler(2.0);
    
    if (servo_port->openPort() && servo_port->setBaudRate(9600)) {
        usleep(2000000);
        
        uint8_t error = 0;
        
        // Проверяем сервопривод
        uint16_t model;
        if (servo_packet->ping(servo_port, SERVO_ID, &model, &error) == COMM_SUCCESS) {
            std::cout << "   ? Servo found (Model: " << model << ")" << std::endl;
        } else {
            std::cout << "   ? Servo not found!" << std::endl;
            servo_port->closePort();
            return -1;
        }
        
        // Настраиваем сервопривод
        servo_packet->write1ByteTxRx(servo_port, SERVO_ID, TORQUE_ENABLE_2, 1, &error);
        servo_packet->write1ByteTxRx(servo_port, SERVO_ID, OPERATING_MODE_2, 1, &error);
        servo_packet->write4ByteTxRx(servo_port, SERVO_ID, GOAL_VELOCITY_2, 200, &error);
        
        std::cout << "   ??  Servo started RIGHT\n" << std::endl;
        
        servo_port->closePort();
    }
    
    // 2. Инициализация светодиодов (протокол 1.0, 57600)
    std::cout << "2. Initializing LEDs..." << std::endl;
    
    PortHandler* led_port = PortHandler::getPortHandler(PORT_NAME);
    PacketHandler* led_packet = PacketHandler::getPacketHandler(1.0);
    
    if (led_port->openPort() && led_port->setBaudRate(57600)) {
        usleep(1000000);
        
        uint8_t error = 0;
        
        // Проверяем и инициализируем левый светодиод
        uint16_t left_model;
        if (led_packet->ping(led_port, LED_LEFT_ID, &left_model, &error) == COMM_SUCCESS) {
            std::cout << "   ? Left LED found (ID=" << (int)LED_LEFT_ID << ", Model: " << left_model << ")" << std::endl;
        } else {
            std::cout << "   ??  Left LED (ID=" << (int)LED_LEFT_ID << ") not found" << std::endl;
        }
        
        // Проверяем и инициализируем правый светодиод
        uint16_t right_model;
        if (led_packet->ping(led_port, LED_RIGHT_ID, &right_model, &error) == COMM_SUCCESS) {
            std::cout << "   ? Right LED found (ID=" << (int)LED_RIGHT_ID << ", Model: " << right_model << ")" << std::endl;
        } else {
            std::cout << "   ??  Right LED (ID=" << (int)LED_RIGHT_ID << ") not found" << std::endl;
        }
        
        // Включаем правый светодиод (начальное направление - вправо)
        setLEDs(led_packet, led_port, false, true);
        std::cout << "   ?? Right LED ON (direction: RIGHT)\n" << std::endl;
        
        led_port->closePort();
    }
    
    // 3. Основной цикл с выключателем
    std::cout << "3. Starting switch monitoring..." << std::endl;
    std::cout << "   Press switch to toggle direction\n" << std::endl;
    
    while (true) {
        // Читаем выключатель (протокол 1.0, 57600)
        PortHandler* sw_port = PortHandler::getPortHandler(PORT_NAME);
        PacketHandler* sw_packet = PacketHandler::getPacketHandler(1.0);
        
        if (sw_port->openPort() && sw_port->setBaudRate(57600)) {
            usleep(100000);
            
            uint8_t state, error = 0;
            
            // Проверяем выключатель
            if (sw_packet->ping(sw_port, SWITCH_ID, nullptr, &error) != COMM_SUCCESS) {
                std::cout << "Switch error!" << std::endl;
                sw_port->closePort();
                usleep(1000000);
                continue;
            }
            
            // Читаем состояние
            sw_packet->read1ByteTxRx(sw_port, SWITCH_ID, SWITCH_REGISTER_1, &state, &error);
            sw_port->closePort();
            
            bool pressed = (state == 1);
            
            // Если нажали кнопку
            if (pressed && !last_switch) {
                servo_right = !servo_right; // Меняем направление
                
                // Управляем сервоприводом (протокол 2.0, 9600)
                PortHandler* s_port = PortHandler::getPortHandler(PORT_NAME);
                PacketHandler* s_packet = PacketHandler::getPacketHandler(2.0);
                
                if (s_port->openPort() && s_port->setBaudRate(9600)) {
                    usleep(100000);
                    
                    if (servo_right) {
                        std::cout << "?? Switching to RIGHT rotation" << std::endl;
                        s_packet->write4ByteTxRx(s_port, SERVO_ID, GOAL_VELOCITY_2, 200, &error);
                    } else {
                        std::cout << "?? Switching to LEFT rotation" << std::endl;
                        s_packet->write4ByteTxRx(s_port, SERVO_ID, GOAL_VELOCITY_2, -200, &error);
                    }
                    
                    s_port->closePort();
                }
                
                // Управляем светодиодами (протокол 1.0, 57600)
                PortHandler* l_port = PortHandler::getPortHandler(PORT_NAME);
                PacketHandler* l_packet = PacketHandler::getPacketHandler(1.0);
                
                if (l_port->openPort() && l_port->setBaudRate(57600)) {
                    usleep(100000);
                    
                    if (servo_right) {
                        // Вращение вправо: правый светодиод горит, левый выключен
                        setLEDs(l_packet, l_port, false, true);
                        std::cout << "   ?? Right LED ON, Left LED OFF" << std::endl;
                    } else {
                        // Вращение влево: левый светодиод горит, правый выключен
                        setLEDs(l_packet, l_port, true, false);
                        std::cout << "   ?? Left LED ON, Right LED OFF" << std::endl;
                    }
                    
                    l_port->closePort();
                }
                
                // Мигание для обратной связи
                PortHandler* blink_port = PortHandler::getPortHandler(PORT_NAME);
                PacketHandler* blink_packet = PacketHandler::getPacketHandler(1.0);
                
                if (blink_port->openPort() && blink_port->setBaudRate(57600)) {
                    usleep(50000);
                    
                    // Кратковременно выключаем текущий светодиод
                    if (servo_right) {
                        blink_packet->write1ByteTxRx(blink_port, LED_RIGHT_ID, LED_REGISTER_1, 0, &error);
                    } else {
                        blink_packet->write1ByteTxRx(blink_port, LED_LEFT_ID, LED_REGISTER_1, 0, &error);
                    }
                    
                    usleep(50000);
                    
                    // Включаем обратно
                    if (servo_right) {
                        blink_packet->write1ByteTxRx(blink_port, LED_RIGHT_ID, LED_REGISTER_1, 200, &error);
                    } else {
                        blink_packet->write1ByteTxRx(blink_port, LED_LEFT_ID, LED_REGISTER_1, 200, &error);
                    }
                    
                    blink_port->closePort();
                }
            }
            
            last_switch = pressed;
        }
        
        usleep(50000); // 50ms
    }
    
    // Перед выходом выключаем все светодиоды
    PortHandler* cleanup_port = PortHandler::getPortHandler(PORT_NAME);
    PacketHandler* cleanup_packet = PacketHandler::getPacketHandler(1.0);
    
    if (cleanup_port->openPort() && cleanup_port->setBaudRate(57600)) {
        setLEDs(cleanup_packet, cleanup_port, false, false);
        cleanup_port->closePort();
    }
    
    return 0;
}