#include "project.h"

/* ---------- LED helper ---------- */
void SetLED(uint8_t r, uint8_t g, uint8_t b) {
    LED_R_Write(r);
    LED_G_Write(g);
    LED_B_Write(b);
}

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

    uint8_t last_key = 255; // щоб не повторювало

    for(;;)
    {
        readMatrix();

        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 3; j++)
            {
                if(keys[i][j] == 0) // натиснута кнопка
                {
                    uint8 key = keys_map[i][j];

                    if(key != last_key)
                    {
                        last_key = key;

                        /* UART */
                        SW_Tx_UART_PutString("Button ");
                        SW_Tx_UART_PutChar(key + '0');
                        SW_Tx_UART_PutString(" pressed\r\n");

                        /* ---------- COLOR LOGIC ---------- */
                        switch(key)
                        {
                            case 1: case 7: SetLED(0,1,1); break; // Red
                            case 2: case 8: SetLED(1,0,1); break; // Green
                            case 3: case 9: SetLED(1,1,0); break; // Blue
                            case 4: case 10: SetLED(0,0,1); break; // Yellow (*)
                            case 5: case 0: SetLED(0,1,0); break; // Purple
                            case 6: case 11: SetLED(1,0,0); break; // Cyan (#)
                            default: SetLED(1,1,1); break;
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
            SetLED(1,1,1); // вимкнено
        }

        CyDelay(20); // debounce
    }
}