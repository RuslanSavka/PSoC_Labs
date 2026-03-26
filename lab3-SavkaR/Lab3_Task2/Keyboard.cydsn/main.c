/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

/* pointers */
static void (*COLUMN_x_SetDriveMode[3])(uint8 mode) = {
    COLUMN_0_SetDriveMode,
    COLUMN_1_SetDriveMode,
    COLUMN_2_SetDriveMode
};

static void (*COLUMN_x_Write[3])(uint8 value) = {
    COLUMN_0_Write,
    COLUMN_1_Write,
    COLUMN_2_Write
};

static uint8 (*ROW_x_Read[4])() = {
    ROW_0_Read,
    ROW_1_Read,
    ROW_2_Read,
    ROW_3_Read
};

/* key map */
uint8 keys_map[4][3] = {
    {1,2,3},
    {4,5,6},
    {7,8,9},
    {10,0,11} // * = 10, # = 11
};

/* matrix state */
uint8 keys[4][3];

/* init */
static void initMatrix()
{
    for(int column_index = 0; column_index < 3; column_index++)
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ);
}

/* read keypad */

/* read keypad */
static void readMatrix()
{
    uint8_t row_counter = sizeof(ROW_x_Read)/sizeof(ROW_x_Read[0]);
    uint8_t column_counter = sizeof(COLUMN_x_Write)/sizeof(COLUMN_x_Write[0]);
    
    for(int column_index=0; column_index<column_counter; column_index++)
    {
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_STRONG);
        COLUMN_x_Write[column_counter] (0); 

        for(int row_index = 0; row_index < 4; row_index++)
        {
            keys[row_index][column_index] = ROW_x_Read[row_index]();
        }

        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ);
    }
}

/* LED OFF */
void LED_Off()
{
    LED_R_Write(1);
    LED_G_Write(1);
    LED_B_Write(1);
}

/* LED WHITE */
void LED_White()
{
    LED_R_Write(0);
    LED_G_Write(0);
    LED_B_Write(0);
}

/* set color */
void setColor(uint8 key)
{
    LED_Off();
    switch(key)
    {
        case 1: case 7: LED_R_Write(0); break;
        case 2: case 8: LED_G_Write(0); break;
        case 3: case 9: LED_B_Write(0); break;
        case 4: case 10: LED_R_Write(0); LED_G_Write(0); break;
        case 5: case 0: LED_R_Write(0); LED_B_Write(0); break;
        case 6: case 11: LED_G_Write(0); LED_B_Write(0); break;
    }
}

int main(void)
{
    CyGlobalIntEnable;
    
    // Ініціалізація компонентів
    SW_Tx_UART_Start(); 
    initMatrix();

    uint8 last_key = 255;
    char uart_buffer[32]; // буфер для тексту

    LED_White(); 

    for(;;)
    {
        readMatrix();
        uint8 found = 0;

        for(int r = 0; r < 4; r++)
        {
            for(int c = 0; c < 3; c++)
            {
                if(keys[r][c] == 0) 
                {
                    uint8 key = keys_map[r][c];

                    if(last_key != key)
                    {
                        last_key = key;
                        setColor(key);
                        
                        // ВИВІД В КОНСОЛЬ ЧЕРЕЗ PutString
                        switch(key)
                        {
                            case 0:  SW_Tx_UART_PutString("Button 0 pressed");  break;
                            case 1:  SW_Tx_UART_PutString("Button 1 pressed");  break;
                            case 2:  SW_Tx_UART_PutString("Button 2 pressed");  break;
                            case 3:  SW_Tx_UART_PutString("Button 3 pressed");  break;
                            case 4:  SW_Tx_UART_PutString("Button 4 pressed");  break;
                            case 5:  SW_Tx_UART_PutString("Button 5 pressed");  break;
                            case 6:  SW_Tx_UART_PutString("Button 6 pressed");  break;
                            case 7:  SW_Tx_UART_PutString("Button 7 pressed");  break;
                            case 8:  SW_Tx_UART_PutString("Button 8 pressed");  break;
                            case 9:  SW_Tx_UART_PutString("Button 9 pressed");  break;
                            case 10: SW_Tx_UART_PutString("Button * pressed");  break;
                            case 11: SW_Tx_UART_PutString("Button # pressed");  break;
                            default: SW_Tx_UART_PutString("Unknown button");    break;
                        }
                        SW_Tx_UART_PutCRLF(); // Перехід на новий рядок
                    }
                    found = 1;
                }
            }
        }

        if(!found)
        {
            if(last_key != 255)
            {
                last_key = 255;
                LED_Off();
            }
        }
        
        CyDelay(50); // Затримка для стабільної роботи (антибрязк)
    }
}