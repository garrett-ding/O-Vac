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
    FS_Mount(volname);
    if(0 != FS_GetVolumeName(0u, volname, 9u))
    {
        /* Getting volume name succeeded so prompt it on the LCD */
        LED4_Write(1);
        CyDelay(500u);
    }
    else
        return 1;   
    
    CyDelay(500u);
    LED4_Write(0);
//    setCursor(0u, 0u);
//    LCD_print("SD card format");
    
    if(0 == FS_FormatSD(volname))
    {
        LED4_Write(1);
//        setCursor(1u, 0u);
//        LCD_print("Succeeded");
    }
    else
    {
//        setCursor(1u, 0u);
//        LCD_print("Failed");
        return 1;
    }
    CyDelay(500u);
    LED4_Write(0);
    file = FS_FOpen(filename, "w");
    if(file)
    {
        /* Indicate successful file creation message */
//        LCD_print("File ");
//        LCD_print(filename);
//        setCursor(1u, 0u);
//        LCD_print("was opened");
        LED6_Write(1);
        /* Need some delay to indicate output on the LCD */
        CyDelay(1000u);
        LED6_Write(0);
//        clear();
        
//        setCursor(0u, 0u);
        
        if(0 != FS_Write(file, "Testing Testing\nTesting Testing", 31u)) 
        {
            /* Inditate that data was written to a file */
//            LCD_print("\"0123456789\"");
//            LCD_print(filename);
//            setCursor(1u, 0u);
//            LCD_print("written to file");
            LED7_Write(1);
            CyDelay(1000u);
            LED7_Write(0);
        }
        else
        {
//            LCD_print("Failed to write");
//            setCursor(1u, 0u);
//            LCD_print("data to file");
            return 1;
        }
    }
    else
    {
//        LCD_print("Failed to create");
//        setCursor(1u, 0u);
//        LCD_print("file");
        return 1;
    }
    return 0;
}

/* [] END OF FILE */
