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
    {10,0,11} // * = 10, # = 11
};

/* matrix state */
uint8 keys[4][3];

/* init */
void initMatrix()
{
    for(int i=0;i<3;i++)
        COLUMN_x_SetDriveMode[i](COLUMN_0_DM_DIG_HIZ);
}

/* read keypad */

/* read keypad */
void readMatrix()
{
    for(int c=0;c<3;c++)
    {
        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_STRONG);
        COLUMN_x_Write; 

        for(int r=0; r<4; r++)
        {
            keys[r][c] = ROW_x_Read[r]();
        }

        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_DIG_HIZ);
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
        case 1:
        case 7:
            LED_R_Write(0);
            break;

        case 2:
        case 8:
            LED_G_Write(0);
            break;

        case 3:
        case 9:
            LED_B_Write(0);
            break;

        case 4:
        case 10: // *
            LED_R_Write(0);
            LED_G_Write(0);
            break;

        case 5:
        case 0:
            LED_R_Write(0);
            LED_B_Write(0);
            break;

        case 6:
        case 11: // #
            LED_G_Write(0);
            LED_B_Write(0);
            break;
    }
}

/* check if any button pressed */
uint8 isPressed()
{
    for(int r=0;r<4;r++)
        for(int c=0;c<3;c++)
            if(keys[r][c] == 0)
                return 1;

    return 0;
}

int main(void)
{
    CyGlobalIntEnable;

    initMatrix();

    uint8 last_key = 255;

    LED_White(); // стартовий стан

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
                        setColor(key);
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
                LED_Off(); // відпустив → вимкнути
            }
        }
    }
}
