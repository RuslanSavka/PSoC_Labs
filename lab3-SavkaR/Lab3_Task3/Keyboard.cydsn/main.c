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
static void (*COLUMN_x_SetDriveMode[3])(uint8) = {
    COLUMN_0_SetDriveMode,
    COLUMN_1_SetDriveMode,
    COLUMN_2_SetDriveMode
};

static void (*COLUMN_x_Write[3])(uint8) = {
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

/* PASSWORD */
uint8 password[3] = {1, 2, 3};
uint8 input[10];
uint8 inputIndex = 0;

/* init */
void initMatrix()
{
    for(int i=0;i<3;i++)
        COLUMN_x_SetDriveMode[i](COLUMN_0_DM_DIG_HIZ);
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

        for(int row_index = 0; row_index < 4; row_index++)
        {
            keys[row_index][column_index] = ROW_x_Read[row_index]();
        }

        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ);
    }
}

/* перевірка пароля */
void checkPassword()
{
    if(inputIndex < 3) return;

    if(input[0] == password[0] &&
       input[1] == password[1] &&
       input[2] == password[2])
    {
        SW_Tx_UART_PutString("Access allowed\r\n");
    }
    else
    {
        SW_Tx_UART_PutString("Access denied\r\n");
    }

    inputIndex = 0;
}

int main(void)
{
    CyGlobalIntEnable;

    SW_Tx_UART_Start();
    SW_Tx_UART_PutString("Enter password:\r\n");

    initMatrix();

    uint8 last_key = 255;

    for(;;)
    {
        readMatrix();

        uint8 found = 0;

        for(int r=0;r<4;r++)
        {
            for(int c=0;c<3;c++)
            {
                if(keys[r][c] == 0)
                {
                    uint8 key = keys_map[r][c];

                    if(last_key != key)
                    {
                        last_key = key;

                        SW_Tx_UART_PutString("Pressed: ");
                        SW_Tx_UART_PutHexInt(key);
                        SW_Tx_UART_PutCRLF();

                        if(inputIndex < 10)
                            input[inputIndex++] = key;

                        checkPassword();
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
                SW_Tx_UART_PutString("Button released\r\n");
            }
        }
    }
}