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
#include <string.h>

/* pointers */
static void (*COLUMN_x_SetDriveMode[3])(uint8_t mode) = {
    COLUMN_0_SetDriveMode,
    COLUMN_1_SetDriveMode,
    COLUMN_2_SetDriveMode
};

static void (*COLUMN_x_Write[3])(uint8_t value) = {
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
    {10,0,11}
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
static void readMatrix()
{
    uint8_t row_counter = sizeof(ROW_x_Read)/sizeof(ROW_x_Read[0]);
    uint8_t column_counter = sizeof(COLUMN_x_Write)/sizeof(COLUMN_x_Write[0]);
    
    for(int column_index=0; column_index<column_counter; column_index++)
    {
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_STRONG);
        COLUMN_x_Write[column_counter] (0); 

        for(int row_index = 0; row_index < row_counter; row_index++)
        {
            keys[row_index][column_index] = ROW_x_Read[row_index]();
        }

        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ);
    }
}

int main(void)
{
    CyGlobalIntEnable;
    SW_Tx_UART_Start();

    initMatrix();

    uint8_t last_key = 255;

    /* ---------- PASSWORD ---------- */
    uint8 password[] = {1,2,3,4};
    uint8 input_buf[4];
    uint8 input_idx = 0;

    for(;;)
    {
        readMatrix();

        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 3; j++)
            {
                if(keys[i][j] == 0)
                {
                    uint8 key = keys_map[i][j];

                    if(key != last_key) // фронт натискання
                    {
                        last_key = key;

                        /* показуємо кнопку */
                        SW_Tx_UART_PutString("Pressed: ");
                        SW_Tx_UART_PutChar(key + '0');
                        SW_Tx_UART_PutCRLF();

                        /* запис у буфер */
                        input_buf[input_idx] = key;
                        input_idx++;

                        /* якщо ввели 4 цифри */
                        if(input_idx >= 4)
                        {
                            if(memcmp(input_buf, password, 4) == 0)
                            {
                                SW_Tx_UART_PutString("ACCESS GRANTED\r\n");
                            }
                            else
                            {
                                SW_Tx_UART_PutString("WRONG PASSWORD\r\n");
                            }

                            input_idx = 0; // скидання
                        }
                    }
                }
            }
        }

        /* якщо нічого не натиснуто */
        uint8 any_pressed = 0;
        for(int i=0;i<4;i++)
            for(int j=0;j<3;j++)
                if(keys[i][j]==0) any_pressed = 1;

        if(!any_pressed)
        {
            last_key = 255;
        }

        CyDelay(20);
    }
}