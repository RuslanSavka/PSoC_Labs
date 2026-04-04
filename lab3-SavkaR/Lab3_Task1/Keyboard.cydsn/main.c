#include "project.h"
#include <string.h>

void SetLED(uint8_t r, uint8_t g, uint8_t b) {
    LED_R_Write(r);
    LED_G_Write(g);
    LED_B_Write(b);
}
char ScanKeypad(void) {
    char key = 0;

    C_1_Write(0); C_2_Write(1); C_3_Write(1);
    CyDelayUs(10);
    if (R_1_Read() == 0) key = '1';
    if (R_2_Read() == 0) key = '4';
    if (R_3_Read() == 0) key = '7';
    if (R_4_Read() == 0) key = '*';

    C_1_Write(1); C_2_Write(0); C_3_Write(1);
    CyDelayUs(10);
    if (R_1_Read() == 0) key = '2';
    if (R_2_Read() == 0) key = '5';
    if (R_3_Read() == 0) key = '8';
    if (R_4_Read() == 0) key = '0';

    C_1_Write(1); C_2_Write(1); C_3_Write(0);
    CyDelayUs(10);
    if (R_1_Read() == 0) key = '3';
    if (R_2_Read() == 0) key = '6';
    if (R_3_Read() == 0) key = '9';
    if (R_4_Read() == 0) key = '#';


    C_1_Write(1); C_2_Write(1); C_3_Write(1);

    return key;
}
int main(void) {
    CyGlobalIntEnable; 

    SW_Tx_UART_Start();
    SW_Tx_UART_PutString("\r\n Start \r\n");

    char key = 0;
    char last_key = 0;
    uint8_t is_started = 0; 

    char password[] = "123"; 
    char input_buf[5] = {0};  
    uint8_t input_idx = 0;

    for(;;) {
        key = ScanKeypad();

        //Завдання 1:
        if (key == 0) {
           
            if (is_started == 0) {
                SetLED(0, 0, 0); 
            } else {
                SetLED(1, 1, 1); 
            }
        } 
        else {
            is_started = 1; 

            //Завдання 2: 
            switch(key) {
                case '1': case '7': SetLED(0, 1, 1); break; 
                case '2': case '8': SetLED(1, 0, 1); break; 
                case '3': case '9': SetLED(1, 1, 0); break; 
                case '4': case '*': SetLED(0, 0, 1); break; 
                case '5': case '0': SetLED(0, 1, 0); break; 
                case '6': case '#': SetLED(1, 0, 0); break; 
                default:            SetLED(1, 1, 1); break; 
            }
        }

        //Завдання 3: Пароль

        if (key != 0 && last_key == 0) {
            input_buf[input_idx] = key;
            input_idx++;
            
            SW_Tx_UART_PutChar(key); 

            if (input_idx >= 3) {
                input_buf[4] = '\0'; 
                
                if (strcmp(input_buf, password) == 0) {
                    SW_Tx_UART_PutString("\r\n >> PASS\r\n");
                } else {
                    SW_Tx_UART_PutString("\r\n >> Fail\r\n");
                }
                
                input_idx = 0; 
            }
        }
        last_key = key; 
        CyDelay(20);    
    }
}