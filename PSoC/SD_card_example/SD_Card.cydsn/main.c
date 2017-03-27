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
#include "functions.h"
#include <FS.h>

CY_ISR_PROTO(write);

int flag = 0;
char sdFile[9] = "File.txt";

int main(void)
{
    char sdVolName[10];
    FS_FILE * pFile;
    Initialize_SD_Card(&sdFile[0], &sdVolName[0], pFile); // in functions.c
    CyDelay(2000u);
    CyGlobalIntEnable; /* Enable global interrupts. */
    LCD_Start(); // LCD start
    Timer_1_Start();    // Timer start
    write_to_StartEx(write); // ISR start

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        if (flag){
            flag = 0;
            FS_Write(pFile, "HERE\n", 5u);
        }
        
        /* Place your application code here. */
    }
}

CY_ISR(write)
{
    flag = 1;
    /* Clear Timer bit to reset counter */
    Timer_1_STATUS;
}
/* [] END OF FILE */
