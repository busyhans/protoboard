//**************************************************************************************
/** \file task_PWM.cpp
 *    This file contains source code for a ramping PWM signal based off an input
 *
 *  Revisions:
 *    \li 10-25-2019 HVH Adapted from HVH task_BUTT.cpp
 *
 *  License:
 *    This file is copyright 2019 by H Hershberger and released under the Shtinky Boi 
 *    Public License, version 2. It intended for educational use only, but its use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *	  (TLDR):  THIS CODE MIGHT SUCK AND YOU'RE ON YOUR OWN  */
//**************************************************************************************
#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>                        // Watchdog timer header

#include "shared_data_sender.h"
#include "shared_data_receiver.h"
#include "task_PWM.h"                      // Header for this file


//-------------------------------------------------------------------------------------
/** This constructor creates a new data acquisition task. Its main job is to call the
 *  parent class's constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_PWM::task_PWM (const char* a_name, 
					  unsigned portBASE_TYPE a_priority, 
					  size_t a_stack_size,
					  emstream* p_ser_dev
					 )
	: frt_task (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. All the work is done in the
	// call to the frt_task constructor on the line just above this one
}


//-------------------------------------------------------------------------------------
/** This task ramps up a PWM signal saturated at 100% based of an input
 */

void task_PWM::run (void)
{
	// Make a variable which will hold times to use for precise task scheduling
	portTickType previousTicks = xTaskGetTickCount ();

	// Wait a little while for user interface task to finish up
	delay_ms(10);
	
	while(1)
	{
		switch (state)
		{
		case INIT:
			
			// SETTING UP PE1 and PE0 (EXT1 PIN7 and PIN8)
			pwm_ctr = 0;						// Clear the counter
			PORTE.OUTCLR = PIN1_bm | PIN0_bm;				// Make sure the pin is off before configuring it as output
			PORTE.DIRSET = PIN1_bm | PIN0_bm;				// Set the pin as an output
			PORTE.OUTSET = PIN1_bm | PIN0_bm;				// Turn the pin on again
			// SETTING UP TIMER
			TCE0_CTRLB = TC_WGMODE_SS_gc | TC0_CCBEN_bm | TC0_CCAEN_bm;	// single slope, compare enable to B because Port D5
			TCE0_PER = 1600;					// Set period to 1600
			TCE0_CCABUF = 400;					// Set compare to 0
			TCE0_CCBBUF = 800;					// Set compare to 0
			TCE0_CTRLD = 0;						// All event stuff off
			TCE0_CTRLC = 0;						// timer counter is always on
			TCE0_CTRLA |= TC_CLKSEL_DIV1_gc;		// Prescaler is just clock frequency
	
			transition_to(PWM_OFF);				// Go to checking for pwm off state
			break;
			
		case PWM_OFF:
			//TCE0_CCB = 0;						// Set compare to 0
			//TCE0_CCA = 800;						// Set compare to 0
			/*
			if(butt_flg.get()==true)			// If button pushed
			{
				pwm_ctr = 5;
				transition_to(PWM_RAMP);
			}
			*/
			break;
			
		case PWM_RAMP:
			if(pwm_ctr == 0  &&  butt_flg.get() == true && TCE0_CCB < 1600)
			{
				TCE0_CCB += 160;
				if(TCE0_CCB > 1600)
				{
					TCE0_CCB = 1600;
				}
				pwm_ctr++;
			}
			else if(butt_flg.get() == false)				// If button released
			{	
				pwm_ctr = 0;
				transition_to(PWM_RUN);
			}
			else if(pwm_ctr == 15)
			{
				pwm_ctr = 0;
			}
			else
			{
				pwm_ctr++;
			}
			break;
			
		case PWM_RUN:
			if(butt_flg.get()==true)				// If button pushed
				{
					transition_to(PWM_OFF);
				}
			break;
		
		default:
			break;
		}
		runs++;
		delay_from_to_ms(previousTicks,10);
	}
}