/*******************************************************************************
* File Name: write_to.h
* Version 1.70
*
*  Description:
*   Provides the function definitions for the Interrupt Controller.
*
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(CY_ISR_write_to_H)
#define CY_ISR_write_to_H


#include <cytypes.h>
#include <cyfitter.h>

/* Interrupt Controller API. */
void write_to_Start(void);
void write_to_StartEx(cyisraddress address);
void write_to_Stop(void);

CY_ISR_PROTO(write_to_Interrupt);

void write_to_SetVector(cyisraddress address);
cyisraddress write_to_GetVector(void);

void write_to_SetPriority(uint8 priority);
uint8 write_to_GetPriority(void);

void write_to_Enable(void);
uint8 write_to_GetState(void);
void write_to_Disable(void);

void write_to_SetPending(void);
void write_to_ClearPending(void);


/* Interrupt Controller Constants */

/* Address of the INTC.VECT[x] register that contains the Address of the write_to ISR. */
#define write_to_INTC_VECTOR            ((reg32 *) write_to__INTC_VECT)

/* Address of the write_to ISR priority. */
#define write_to_INTC_PRIOR             ((reg8 *) write_to__INTC_PRIOR_REG)

/* Priority of the write_to interrupt. */
#define write_to_INTC_PRIOR_NUMBER      write_to__INTC_PRIOR_NUM

/* Address of the INTC.SET_EN[x] byte to bit enable write_to interrupt. */
#define write_to_INTC_SET_EN            ((reg32 *) write_to__INTC_SET_EN_REG)

/* Address of the INTC.CLR_EN[x] register to bit clear the write_to interrupt. */
#define write_to_INTC_CLR_EN            ((reg32 *) write_to__INTC_CLR_EN_REG)

/* Address of the INTC.SET_PD[x] register to set the write_to interrupt state to pending. */
#define write_to_INTC_SET_PD            ((reg32 *) write_to__INTC_SET_PD_REG)

/* Address of the INTC.CLR_PD[x] register to clear the write_to interrupt. */
#define write_to_INTC_CLR_PD            ((reg32 *) write_to__INTC_CLR_PD_REG)


#endif /* CY_ISR_write_to_H */


/* [] END OF FILE */
