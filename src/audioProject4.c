#include "address_map_arm.h"


/* globals */
#define BUF_SIZE 480000			// about 10 seconds of buffer (@ 48K samples/sec)
#define BUF_THRESHOLD 0		// 75% of 128 word buffer  : 96


void check_KEYs( int *, int *, int *, int *, int * );
void check_SW(int *);



/********************************************************************************
 * This program demonstrates use of the media ports in the DE1-SoC Computer
 *
 * It performs the following: 
 *  	1. records audio for about 10 seconds when KEY[0] is pressed. LEDR[0] is
 *  	   lit while recording
 * 	2. plays the recorded audio when KEY[1] is pressed. LEDR[1] is lit while 
 * 	   playing
 * 	3. Draws a blue box on the VGA display, and places a text string inside
 * 	   the box
 * 	4. Displays the last three bytes of data received from the PS/2 port 
 * 	   on the HEX displays on the DE1-SoC Computer
********************************************************************************/
int main(void)
{
	/* Declare volatile pointers to I/O registers (volatile means that IO load
	   and store instructions will be used to access these pointer locations, 
	   instead of regular memory loads and stores) */
  	volatile int * red_LED_ptr = (int *) LEDR_BASE;
	volatile int * audio_ptr = (int *) AUDIO_BASE;
	volatile int * key_ptr = (int *) KEY_BASE;
//	volatile int * PS2_ptr = (int *) PS2_BASE;

	/* used for audio record/playback */
	int fifospace;
	int record = 0, play = 0, buffer_index = 0;
	int velocity = 0, reverse = 0;
	int left_buffer[BUF_SIZE];
	int right_buffer[BUF_SIZE];
	int vol = 0x7fffffff;
	int sw_value = 0;
	/* read and echo audio data */
	record = 0;
	play = 0;
	
	// PS/2 mouse needs to be reset (must be already plugged in)
//	*(PS2_ptr) = 0xFF;		// reset
	while(1)
	{
		check_KEYs ( &record, &play, &velocity, &reverse, &buffer_index);
		check_SW(&sw_value);
		if (record)
		{
			*(red_LED_ptr) = 0x1;					// turn on LEDR[0]
			fifospace = *(audio_ptr + 1);	 			// read the audio port fifospace register
			if ( (fifospace & 0x000000FF) > BUF_THRESHOLD ) 	// check RARC
			{
				// store data until the the audio-in FIFO is empty or the buffer is full
				while ( (fifospace & 0x000000FF) && (buffer_index < BUF_SIZE) )
				{	
					
					
					left_buffer[buffer_index] = *(audio_ptr + 2); 		
					right_buffer[buffer_index] = *(audio_ptr + 3); 		
					++buffer_index;

					if (buffer_index == BUF_SIZE)
					{
						// done recording
						record = 0;
						*(red_LED_ptr) = 0x0;				// turn off LEDR
					}
					fifospace = *(audio_ptr + 1);	// read the audio port fifospace register
				}
			}
		}
		else if (play)
		{
			*(red_LED_ptr) = 0x2;					// turn on LEDR_1
			fifospace = *(audio_ptr + 1);	 		// read the audio port fifospace register
			if ( (fifospace & 0x00FF0000) > BUF_THRESHOLD ) 	// check WSRC
			{
				// output data until the buffer is empty or the audio-out FIFO is full
				while ( (fifospace & 0x00FF0000) && (buffer_index < BUF_SIZE) )
				{
					
					*(audio_ptr + 2) = left_buffer[buffer_index];
					*(audio_ptr + 3) = right_buffer[buffer_index];

					++buffer_index;

					if (buffer_index == BUF_SIZE)
					{
						// done playback
						play = 0;
						*(red_LED_ptr) = 0x0;				// turn off LEDR
					}
					fifospace = *(audio_ptr + 1);	// read the audio port fifospace register
				}
			}
		}
		else if (velocity)
		{	
			if(sw_value){ //더 빠르게 
				(red_LED_ptr) = 0x4;					// turn on LEDR_1
				fifospace = *(audio_ptr + 1);	 		// read the audio port fifospace register
				if ( (fifospace & 0x00FF0000) > BUF_THRESHOLD ) 	// check WSRC
				{
					// output data until the buffer is empty or the audio-out FIFO is full
					while ( (fifospace & 0x00FF0000) && (buffer_index < BUF_SIZE / 2) )
					{
			
						int index = buffer_index * 2;
						*(audio_ptr + 2) = left_buffer[index];
						*(audio_ptr + 3) = right_buffer[index];

						++buffer_index;

						if (buffer_index == BUF_SIZE / 2)
						{
							// done playback
							velocity = 0;
							*(red_LED_ptr) = 0x0;				// turn off LEDR
						}
						fifospace = *(audio_ptr + 1);	// read the audio port fifospace register
					}
				}
			}
			else{ //더 느리게
				(red_LED_ptr) = 0x4;					// turn on LEDR_1
				fifospace = *(audio_ptr + 1);	 		// read the audio port fifospace register
				if ( (fifospace & 0x00FF0000) > BUF_THRESHOLD ) 	// check WSRC
				{	
					int mul = 0;
					// output data until the buffer is empty or the audio-out FIFO is full
					while ( (fifospace & 0x00FF0000) && (buffer_index < BUF_SIZE * 2) )
					{

						int index = buffer_index/2;
						*(audio_ptr + 2) = left_buffer[index];
						*(audio_ptr + 3) = right_buffer[index];		

						++buffer_index;	

						if (buffer_index >= BUF_SIZE/2)
						{
							velocity = 0;
							*(red_LED_ptr) = 0x0;				// turn off LEDR
						}
						fifospace = *(audio_ptr + 1);	// read the audio port fifospace register
					}
				}
			}
			
		}
		else if (reverse)
		{
			*(red_LED_ptr) = 0x8;					// turn on LEDR_1
			fifospace = *(audio_ptr + 1);	 		// read the audio port fifospace register
			if ( (fifospace & 0x00FF0000) > BUF_THRESHOLD ) 	// check WSRC
			{
				// output data until the buffer is empty or the audio-out FIFO is full
				while ( (fifospace & 0x00FF0000) && (buffer_index < BUF_SIZE) )
				{
					
					*(audio_ptr + 2) = left_buffer[BUF_SIZE - buffer_index -1];
					*(audio_ptr + 3) = right_buffer[BUF_SIZE - buffer_index -1];
					
					++buffer_index;

					if (buffer_index == BUF_SIZE)
					{
						reverse = 0;
						*(red_LED_ptr) = 0x0;				// turn off LEDR
					}
					fifospace = *(audio_ptr + 1);	// read the audio port fifospace register
				}
			}
		}
	}
}


void check_KEYs(int * KEY0, int * KEY1, int * KEY2, int * KEY3, int * counter)
{
	volatile int * KEY_ptr = (int *) KEY_BASE;
	volatile int * audio_ptr = (int *) AUDIO_BASE;
	int KEY_value;

	KEY_value = *(KEY_ptr); 				// read the pushbutton KEY values
	while (*KEY_ptr);							// wait for pushbutton KEY release

	if (KEY_value == 0x1)					// check KEY0
	{
		// reset counter to start recording
		*counter = 0;
		// clear audio-in FIFO
		*(audio_ptr) = 0x4;
		*(audio_ptr) = 0x0;

		*KEY0 = 1;
	}
	else if (KEY_value == 0x2)				// check KEY1
	{
		// reset counter to start playback
		*counter = 0;
		// clear audio-out FIFO
		*(audio_ptr) = 0x8;
		*(audio_ptr) = 0x0;

		*KEY1 = 1;
	}
	else if (KEY_value == 0x4)				// check KEY1
	{
		// reset counter to start playback
		*counter = 0;
		// clear audio-out FIFO
		*(audio_ptr) = 0x8;
		*(audio_ptr) = 0x0;

		*KEY2 = 1;
	}
	else if (KEY_value == 0x8)				// check KEY1
	{
		// reset counter to start playback
		*counter = 0;
		// clear audio-out FIFO
		*(audio_ptr) = 0x8;
		*(audio_ptr) = 0x0;

		*KEY3 = 1;
	}

}

void check_SW(int * value){  // 1 2 4 8 16 -> 1.2 1.4 1.6 1.8 2.0   //  32 64 128 256 512  --> 
	volatile int * sw_ptr = (int *) SW_BASE;
	*value = *(sw_ptr);
}
