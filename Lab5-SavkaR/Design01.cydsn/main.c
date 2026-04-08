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

/* --- СИМВОЛИ --- */
static uint8_t LED_NUM[] = {
    0xC0, 0xF9, 0xA4, 0xB0,
    0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0xBF, 0x88,
    0x83, 0xC6, 0xA1, 0x86,
    0x8E, 0x7F
};

/* --- ДИСПЛЕЙ --- */
uint8_t display[8] = {5,6,0,0,0,0,0,0};
uint8_t current_digit = 0;

/* --- ЗСУВ --- */
uint16_t shift_counter = 0;

/* --- ТАЙМЕР 5 ХВ --- */
uint32_t ms_counter = 0;
uint16_t seconds = 0;
uint8_t minutes = 0;

/* --- ПАРОЛЬ --- */
uint8_t password[4] = {1,2,3,4};
uint8_t input[4];
uint8_t input_index = 0;
uint8_t input_mode = 0;

/* --- КЛАВІАТУРА --- */
static uint8_t keys[4][3];

static void (*COLUMN_x_SetDriveMode[3])(uint8_t) = {
    C_1_SetDriveMode, C_2_SetDriveMode, C_3_SetDriveMode
};

static void (*COLUMN_x_Write[3])(uint8_t) = {
    C_1_Write, C_2_Write, C_3_Write
};

static uint8 (*ROW_x_Read[4])() = {
    R_1_Read, R_2_Read, R_3_Read, R_4_Read
};

/* --- ІНІЦІАЛІЗАЦІЯ КЛАВІАТУРИ --- */
void initMatrix()
{
    for(int i = 0; i < 3; i++)
        COLUMN_x_SetDriveMode[i](C_1_DM_DIG_HIZ);
}

/* --- ЧИТАННЯ КЛАВІАТУРИ --- */
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

/* --- SEND --- */
void sendData(uint8_t data)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        Pin_DO_Write(data & (0x80 >> i));
        Pin_CLK_Write(1);
        Pin_CLK_Write(0);
    }
}

void sendOneDigit(uint8_t pos, uint8_t digit)
{
    sendData(0xFF & ~(1 << pos));
    sendData(LED_NUM[digit]);
    Pin_Latch_Write(1);
    Pin_Latch_Write(0);
}

/* --- ЗСУВ --- */
void shiftDisplay()
{
    uint8_t t1 = display[3];
    display[3]=display[2];
    display[2]=display[1];
    display[1]=display[0];
    display[0]=t1;

    uint8_t t2 = display[4];
    display[4]=display[5];
    display[5]=display[6];
    display[6]=display[7];
    display[7]=t2;
}

/* --- МИГАННЯ --- */
void blinkOK()
{
    for(int i=0;i<8;i++) display[i]=8;
}

void blinkERR()
{
    for(int i=0;i<8;i++) display[i]=0;
}

/* --- ПЕРЕВІРКА ПАРОЛЮ --- */
void checkPassword()
{
    uint8_t ok = 1;
    for(int i=0;i<4;i++)
        if(input[i] != password[i]) ok = 0;

    if(ok) blinkOK();
    else blinkERR();
}

/* --- ОБРОБКА КНОПОК --- */
void handleKeyboard()
{
    readMatrix();

    if(keys[3][2] == 0) // #
    {
        input_mode = 1;
        input_index = 0;
    }

    if(input_mode)
    {
        for(int r=0;r<4;r++)
        {
            for(int c=0;c<3;c++)
            {
                if(keys[r][c] == 0)
                {
                    uint8_t val;

                    if(r==3 && c==0) val=10; // *
                    else if(r==3 && c==1) val=0;
                    else val = r*3 + c + 1;

                    if(val <= 9 && input_index < 4)
                    {
                        input[input_index++] = val;
                        display[input_index-1] = val;
                    }
                }
            }
        }

        if(input_index == 4)
        {
            checkPassword();
            input_mode = 0;
        }
    }
}

/* --- ISR --- */
CY_ISR(Timer_ISR)
{
    sendOneDigit(current_digit, display[current_digit]);

    current_digit++;
    if(current_digit >= 8) current_digit = 0;

    shift_counter++;
    if(shift_counter >= 500)
    {
        shift_counter = 0;
        shiftDisplay();
    }

    /* таймер */
    ms_counter++;
    if(ms_counter >= 1000)
    {
        ms_counter = 0;
        seconds++;

        if(seconds >= 60)
        {
            seconds = 0;
            minutes++;
        }

        /* показ таймера на перших 4 */
        display[0] = minutes / 10;
        display[1] = minutes % 10;
        display[2] = seconds / 10;
        display[3] = seconds % 10;

        if(minutes >= 5)
        {
            blinkOK();
        }
    }
}

/* --- MAIN --- */
int main(void)
{
    CyGlobalIntEnable;

    initMatrix();

    Timer_Int_StartEx(Timer_ISR);
    Timer_Start();

    for(;;)
    {
        handleKeyboard();
    }
}