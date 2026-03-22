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

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    for(;;)
    {
        // Кнопка НЕ натиснута
        if(Button_Read() == 1)
        {
            // СИНІЙ (тільки B)
            LED_R_Write(1);
            LED_G_Write(1);
            LED_B_Write(0); // ON

            CyDelay(500);

            // Вимкнути
            LED_B_Write(1);

            CyDelay(500);
        }
        else // Кнопка натиснута
        {
            // БІЛИЙ (R + G + B)
            LED_R_Write(0);
            LED_G_Write(0);
            LED_B_Write(0);

            CyDelay(500);

            // Вимкнути все
            LED_R_Write(1);
            LED_G_Write(1);
            LED_B_Write(1);

            CyDelay(500);
        }
    }
}