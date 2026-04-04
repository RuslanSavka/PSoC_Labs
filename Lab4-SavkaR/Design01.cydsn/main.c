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
/* */
static uint8_t LED_NUM[] = {
    0xC0, //0
    0xF9, //1
    0xA4, //2
    0xB0, //3
    0x99, //4
    0x92, //5
    0x82, //6
    0xF8, //7
    0x80, //8
    0x90, //9
    0xBF, //-
    0x88, //A
    0x83, //b
    0xC6, //C
    0xA1, //d
    0x86, //E
    0x8E, //F
    0x7F  //.
};
/* send data function */
static void FourDigit74HC595_sendData(uint8_t data) {
    for(uint8_t i = 0; i < 8; i++) {
        if(data & (0x80 >> i))
        {
            Pin_DO_Write(1);
        }
        else
        {
            Pin_DO_Write(0);
        }
        Pin_CLK_Write(1);
        Pin_CLK_Write(0);
        }
}
/* */
static void FourDigit74HC595_sendOneDigit(uint8_t position, uint8_t digit, uint8_t dot)
{
    if(position >= 8) // out of range, clear 7-segment display
    {
        FourDigit74HC595_sendData(0xFF);
        FourDigit74HC595_sendData(0xFF);
    }
    FourDigit74HC595_sendData(0xFF & ~(1 << position));
    if(dot) // if dot is needed
    {
        FourDigit74HC595_sendData(LED_NUM[digit] & 0x7F);
    }
    else
    {
        FourDigit74HC595_sendData(LED_NUM[digit]);
    }
    Pin_Latch_Write(1); // Latch shift register
    Pin_Latch_Write(0);

}
/* [ROW][COLUMN] */
static uint8_t keys[4][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {10, 0, 11},
};

/* arrays of pointers */
/* function of drive mode configuration */
static void (*COLUMN_x_SetDriveMode[3])(uint8_t mode) = {
    C_1_SetDriveMode,
    C_2_SetDriveMode,
    C_3_SetDriveMode
};

/* column write function */
static void (*COLUMN_x_Write[3])(uint8_t value) = {
    C_1_Write,
    C_2_Write,
    C_3_Write
};

/* read row function */
static uint8 (*ROW_x_Read[4])() = {
    R_1_Read,
    R_2_Read,
    R_3_Read,
    R_4_Read
};

/* matrix initialization function */
static void initMatrix()
{
    for(int column_index = 0; column_index < 3; column_index++)
    {
        COLUMN_x_SetDriveMode[column_index](C_1_DM_DIG_HIZ);
    }
}

/* keys matrix read function */
static void readMatrix()
{
    /* define the length of a row and column */
    uint8_t row_counter = sizeof(ROW_x_Read)/sizeof(ROW_x_Read[0]);
    uint8_t column_counter = sizeof(COLUMN_x_Write)/sizeof(COLUMN_x_Write[0]);
    /* column: iterate the columns */
    for(int column_index = 0; column_index < column_counter; column_index++)
    {
        COLUMN_x_SetDriveMode[column_index](C_1_DM_STRONG);
        COLUMN_x_Write[column_index](0);
        /* row: interate throught the rows */
        for(int row_index = 0; row_index < row_counter; row_index++)
        {
            keys[row_index][column_index] = ROW_x_Read[row_index]();
        }
        /* disable the column */
        COLUMN_x_SetDriveMode[column_index](C_1_DM_DIG_HIZ);
    }
}

/* matrix print function */
void printMatrix()
{
    SW_Tx_UART_PutCRLF();
    for (int row_index = 0; row_index < 4; row_index++)
    {
        for(int column_index = 0; column_index < 3; column_index++)
        {
            SW_Tx_UART_PutHexInt(keys[row_index][column_index]);

            SW_Tx_UART_PutString(" ");
        }
        SW_Tx_UART_PutCRLF();
    }
    SW_Tx_UART_PutCRLF();
}

int main(void)
{
    CyGlobalIntEnable;

    SW_Tx_UART_Start();
    initMatrix();

    uint8_t last_state = 255;
    uint8_t mode = 0; // 0 цифри, 1 букви

    for(;;)
    {
        readMatrix();

        /* --- BUTTON 1 --- */
        if(keys[0][0] == 0 && last_state != 1)
        {
            last_state = 1;
            if(mode == 0)
                FourDigit74HC595_sendOneDigit(0, 1, 0);
            else
                FourDigit74HC595_sendOneDigit(0, 11, 0); // A
        }

        /* --- BUTTON 2 --- */
        if(keys[0][1] == 0 && last_state != 2)
        {
            last_state = 2;
            if(mode == 0)
                FourDigit74HC595_sendOneDigit(0, 2, 0);
            else
                FourDigit74HC595_sendOneDigit(0, 12, 0); // b
        }

        /* --- BUTTON 3 --- */
        if(keys[0][2] == 0 && last_state != 3)
        {
            last_state = 3;
            if(mode == 0)
                FourDigit74HC595_sendOneDigit(0, 3, 0);
            else
                FourDigit74HC595_sendOneDigit(0, 13, 0); // C
        }

        /* --- BUTTON 4 --- */
        if(keys[1][0] == 0 && last_state != 4)
        {
            last_state = 4;
            if(mode == 0)
                FourDigit74HC595_sendOneDigit(0, 4, 0);
            else
                FourDigit74HC595_sendOneDigit(0, 14, 0); // d
        }

        /* --- BUTTON 5 --- */
        if(keys[1][1] == 0 && last_state != 5)
        {
            last_state = 5;
            if(mode == 0)
                FourDigit74HC595_sendOneDigit(0, 5, 0);
            else
                FourDigit74HC595_sendOneDigit(0, 15, 0); // E
        }

        /* --- BUTTON 6 --- */
        if(keys[1][2] == 0 && last_state != 6)
        {
            last_state = 6;
            if(mode == 0)
                FourDigit74HC595_sendOneDigit(0, 6, 0);
            else
                FourDigit74HC595_sendOneDigit(0, 16, 0); // F
        }

        /* --- BUTTON 7 --- */
        if(keys[2][0] == 0 && last_state != 7)
        {
            last_state = 7;
            FourDigit74HC595_sendOneDigit(0, 7, 0);
        }

        /* --- BUTTON 8 --- */
        if(keys[2][1] == 0 && last_state != 8)
        {
            last_state = 8;
            FourDigit74HC595_sendOneDigit(0, 8, 0);
        }

        /* --- BUTTON 9 --- */
        if(keys[2][2] == 0 && last_state != 9)
        {
            last_state = 9;
            FourDigit74HC595_sendOneDigit(0, 9, 0);
        }

        /* --- BUTTON * --- */
        if(keys[3][0] == 0 && last_state != 10)
        {
            last_state = 10;
            FourDigit74HC595_sendOneDigit(0, 0, 1);
        }

        /* --- BUTTON 0 --- */
        if(keys[3][1] == 0 && last_state != 0)
        {
            last_state = 0;
            FourDigit74HC595_sendOneDigit(0, 0, 0);
        }

        /* --- BUTTON # (mode switch) --- */
        if(keys[3][2] == 0 && last_state != 11)
        {
            last_state = 11;
            mode = !mode;

            if(mode == 0)
                SW_Tx_UART_PutString("Mode: DIGITS\r\n");
            else
                SW_Tx_UART_PutString("Mode: LETTERS\r\n");
        }

        /* reset state */
        if(keys[0][0] && keys[0][1] && keys[0][2] &&
           keys[1][0] && keys[1][1] && keys[1][2] &&
           keys[2][0] && keys[2][1] && keys[2][2] &&
           keys[3][0] && keys[3][1] && keys[3][2])
        {
            last_state = 255;
        }
    }
}