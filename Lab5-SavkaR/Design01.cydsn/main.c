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

/* SEGMENTS */
static uint8_t LED_NUM[] = {
    0xC0,0xF9,0xA4,0xB0,0x99,
    0x92,0x82,0xF8,0x80,0x90,
    0x8C,0x88,0x86,0x8E,0xC7,
    0xA1,0xC0,0xC8
};

/* DIGITS */
static uint8_t digit_mask[8] = {
    0xFE,0xFD,0xFB,0xF7,
    0xEF,0xDF,0xBF,0x7F
};

uint8_t display_data[8] = {0};
static uint8_t current_digit = 0;

/* MODES */
uint8_t mode = 0;

/* TIMER */
volatile uint8_t one_second_flag = 0;
volatile uint8_t scroll_flag = 0;

uint16_t seconds_left = 300;
uint8_t timer_active = 0;

/* PASSWORD */
#define PASSWORD_LENGTH 4
uint8_t password[PASSWORD_LENGTH] = {1,2,3,4};
uint8_t input[PASSWORD_LENGTH];
uint8_t input_index = 0;

/* SCROLL */
uint8_t scroll_data[4] = {1,2,3,4};
uint8_t fixed_data[4]  = {10,11,5,5};
uint8_t scroll_index = 0;

/* DEBOUNCE */
uint8_t key_pressed[4][3] = {0};

/* SHIFT REGISTER */
void sendData(uint8_t data)
{
    for(uint8_t i=0;i<8;i++)
    {
        Pin_DO_Write((data & (0x80>>i))?1:0);
        Pin_CLK_Write(1);
        Pin_CLK_Write(0);
    }
}

void sendOneDigit(uint8_t pos, uint8_t digit)
{
    if(digit>17) digit=0;

    sendData(0xFF);
    sendData(0xFF);
    Pin_Latch_Write(1);
    Pin_Latch_Write(0);

    sendData(digit_mask[pos]);
    sendData(LED_NUM[digit]);

    Pin_Latch_Write(1);
    Pin_Latch_Write(0);
}

/* DISPLAY */

void showSequence()
{
    uint8_t arr[8]={1,2,3,4,5,6,7,0};
    for(int i=0;i<8;i++) display_data[i]=arr[i];
}

void showTimer(uint16_t sec)
{
    uint8_t min=sec/60;
    uint8_t s=sec%60;

    display_data[0]=min/10;
    display_data[1]=min%10;
    display_data[2]=s/10;
    display_data[3]=s%10;

    for(int i=4;i<8;i++) display_data[i]=0;
}

void showDONE()
{
    display_data[0]=15;
    display_data[1]=0;
    display_data[2]=17;
    display_data[3]=12;

    for(int i=4;i<8;i++) display_data[i]=0;
}

void showInput()
{
    for(int i=0;i<8;i++) display_data[i]=0;

    for(int i=0;i<input_index;i++)
        display_data[7-i]=input[input_index-1-i];
}

void showPASS()
{
    for(int i=0;i<8;i++) display_data[i]=0;

    display_data[4]=10;
    display_data[5]=11;
    display_data[6]=5;
    display_data[7]=5;
}

void showFAIL()
{
    for(int i=0;i<8;i++) display_data[i]=0;

    display_data[4]=13;
    display_data[5]=11;
    display_data[6]=1;
    display_data[7]=14;
}

void showScroll()
{
    for(int i=0;i<4;i++)
        display_data[i]=scroll_data[(i+scroll_index)%4];

    for(int i=0;i<4;i++)
        display_data[i+4]=fixed_data[i];
}

/* PASSWORD */
void checkPassword()
{
    uint8_t ok=1;

    for(int i=0;i<PASSWORD_LENGTH;i++)
        if(input[i]!=password[i])
            ok=0;

    if(ok) showPASS();
    else showFAIL();

    input_index=0;
}

/* ISR */
CY_ISR(Timer_ISR)
{
    static uint16_t tick=0;

    /* display */
    sendOneDigit(current_digit, display_data[current_digit]);

    current_digit++;
    if(current_digit>=8) current_digit=0;

    tick++;

    /* scroll ~ швидкий */
    if(tick % 500 == 0)
        scroll_flag = 1;

    /* 1 секунда */
    if(tick >= 1000)
    {
        tick=0;
        one_second_flag=1;
    }
}

/* MATRIX */
static uint8_t keys[4][3];

static void (*COLUMN_x_SetDriveMode[3])(uint8_t)={
    C_1_SetDriveMode,C_2_SetDriveMode,C_3_SetDriveMode
};

static void (*COLUMN_x_Write[3])(uint8_t)={
    C_1_Write,C_2_Write,C_3_Write
};

static uint8 (*ROW_x_Read[4])()={
    R_1_Read,R_2_Read,R_3_Read,R_4_Read
};

void initMatrix()
{
    for(int i=0;i<3;i++)
        COLUMN_x_SetDriveMode[i](C_1_DM_DIG_HIZ);
}

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

/* MAIN */
int main(void)
{
    CyGlobalIntEnable;

    Timer_Int_StartEx(Timer_ISR);
    Timer_Start();

    initMatrix();

    uint8_t last=255;

    showSequence();

    for(;;)
    {
        readMatrix();

        /* debounce reset */
        for(int r=0;r<4;r++)
        for(int c=0;c<3;c++)
            if(keys[r][c]!=0)
                key_pressed[r][c]=0;

        /* mode switch */
        if(keys[3][2]==0 && last!=11 && !key_pressed[3][2])
        {
            last=11;
            key_pressed[3][2]=1;

            mode=(mode+1)%4;

            /* FIX */
            timer_active=0;
            one_second_flag=0;

            if(mode==0) showSequence();

            if(mode==1)
            {
                seconds_left=300;
                timer_active=1;
                showTimer(seconds_left);
            }

            if(mode==2)
            {
                input_index=0;
                for(int i=0;i<8;i++) display_data[i]=0;
            }

            if(mode==3)
            {
                scroll_index=0;
                showScroll();
            }
        }

        /* TIMER */
        if(mode==1 && timer_active && one_second_flag)
        {
            one_second_flag=0;

            if(seconds_left>0)
            {
                seconds_left--;
                showTimer(seconds_left);
            }
            else
            {
                timer_active=0;
                showDONE();
            }
        }

        /* SCROLL */
        if(mode==3 && scroll_flag)
        {
            scroll_flag=0;

            scroll_index++;
            if(scroll_index>=4) scroll_index=0;

            showScroll();
        }

        /* PASSWORD */
        if(mode==2)
        {
            for(int r=0;r<4;r++)
            for(int c=0;c<3;c++)
            {
                if(keys[r][c]==0 && !key_pressed[r][c])
                {
                    key_pressed[r][c]=1;

                    uint8_t val;

                    if(r==3 && c==1) val=0;
                    else val=r*3+c+1;

                    if(val<=9)
                    {
                        if(input_index<PASSWORD_LENGTH)
                        {
                            input[input_index++]=val;
                            showInput();
                        }

                        if(input_index==PASSWORD_LENGTH)
                            checkPassword();
                    }
                }
            }
        }

        /* reset */
        if(keys[0][0] && keys[0][1] && keys[0][2] &&
           keys[1][0] && keys[1][1] && keys[1][2] &&
           keys[2][0] && keys[2][1] && keys[2][2] &&
           keys[3][0] && keys[3][1] && keys[3][2])
        {
            last=255;
        }
    }
}