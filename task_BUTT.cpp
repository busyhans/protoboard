//**************************************************************************************
/** \file task_BUTT.cpp
 *    This file contains source code for a user interface task with a single push button
 *
 *  Revisions:
 *    \li 10-25-2019 HVH Adapted from JRR task_LED.cpp
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
#include "task_BUTT.h"                      // Header for this file


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

task_BUTT::task_BUTT (const char* a_name, 
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
/** This task inputs button pressed attached to PORTQ Pin 2
 */

// Create butt_flg shared boolean flag
shared_data<bool> butt_flg;

void task_BUTT::run (void)
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
			PORTQ.OUTCLR = PIN2_bm;				// Make sure the pin is off before configuring it as output
			PORTQ.DIRCLR = PIN2_bm;				// Set the pin as an input
			PORTQ.PIN2CTRL = PORT_OPC_PULLUP_gc; // Set pullup resistor
			PORTQ.OUTSET = PIN2_bm;				// Turn the pin on again
			transition_to(BUTT_CHECK);			// Go to checking for button state
			break;
			
		case BUTT_CHECK:
			if(!(PORTQ.IN & PIN2_bm))			// If button pushed (input low) and we are looking at pin 2 (supposed to be &)
			{
				butt_flg.put(true);				// Hey everybody, button is pushed
				transition_to(BUTT_PRESS);		// Go to button is pressed state
			}
			break;
			
		case BUTT_PRESS:
			if(PORTQ.IN & PIN2_bm)				// If button released (input high) and we are looking at pin 2 (high) (supposed to be &)
			{
				butt_flg.put(false);			// Hey everybody, button is released
				transition_to(BUTT_CHECK);		// Go back to checking for button pressed
			}
			break;
		
		default:
			break;
		}
		runs++;
		delay_from_to_ms(previousTicks,10);
	}
}