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
/* Source file for helper functions, make main a little shorter*/

#include "functions.h"

int Initialize_SD_Card(char *filename, char *volname, FS_FILE *file){
    FS_Init();
    if(0 != FS_GetVolumeName(0u, volname, 9u))
    {
        /* Getting volume name succeeded so prompt it on the LCD */
        LCD_PrintString("SD card name:");
        LCD_Position(1u, 0u);
        LCD_PrintString(volname);
    }
    else
    {
        /* Operation Failed - indicate this */
        LCD_PrintString("Failed to get");
        LCD_Position(1u, 0u);
        LCD_PrintString("SD card name");
        return 1;
    }
    CyDelay(500u);
    LCD_Position(0u, 0u);
    LCD_PrintString("SD card format");
    
    if(0 == FS_FormatSD(volname))
    {
        LCD_Position(1u, 0u);
        LCD_PrintString("Succeeded");
    }
    else
    {
        LCD_Position(1u, 0u);
        LCD_PrintString("Failed");
        return 1;
    }
    CyDelay(500u);
    file = FS_FOpen(filename, "w");
    if(file)
    {
        /* Indicate successful file creation message */
        LCD_PrintString("File ");
        LCD_PrintString(filename);
        LCD_Position(1u, 0u);
        LCD_PrintString("was opened");
        
        /* Need some delay to indicate output on the LCD */
        CyDelay(500u);
        
        LCD_ClearDisplay();
        
        LCD_Position(0u, 0u);
        
        if(0 != FS_Write(file, "Test Test\n", 10u)) 
        {
            /* Inditate that data was written to a file */
            LCD_PrintString("\"0123456789\"");
            LCD_PrintString(filename);
            LCD_Position(1u, 0u);
            LCD_PrintString("written to file");
        }
        else
        {
            LCD_PrintString("Failed to write");
            LCD_Position(1u, 0u);
            LCD_PrintString("data to file");
            return 1;
        }
    }
    else
    {
        LCD_PrintString("Failed to create");
        LCD_Position(1u, 0u);
        LCD_PrintString("file");
        return 1;
    }
    return 0;
}

/* [] END OF FILE */
