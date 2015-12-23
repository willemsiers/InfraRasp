#include <wiringPi.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#define GPIO_IR_RECEIVER 2
#define END_BYTE 168

unsigned char received_bytes[300*1024];
unsigned int received_bits_total = 0;
unsigned int received_bytes_total = 0;
unsigned int received_checksum = 0;

unsigned char* current_receiving_byte = NULL;

int
done_receive_byte()
{
	printf("rec byte\t%c",*current_receiving_byte);
	received_checksum += *current_receiving_byte;
	printf("receive checksum %u\n", received_checksum);
	
	if(*current_receiving_byte == '\0')
	{
		return 1; //return 'done'
	}

	current_receiving_byte = &received_bytes[ ++ received_bytes_total]; 

	return 0; //return 'not done yet'
}

int
receive_bit(unsigned char bit)
{
	int done = 0;
	unsigned char bit_in_place = bit << (7 - (received_bits_total % 8));
	(*current_receiving_byte) |= bit_in_place;

	received_bits_total++;
	if(received_bits_total % 8 == 0)
	{
		done = done_receive_byte();
	}
	return done;
}


int main(int argc, char *argv[]){
	setbuf(stdout, NULL);
	if(argc <= 1){
		fprintf(stderr, "provide a transmission interval argument (delay)");
		return(1);
	}

	const int TRANSMISSION_INTERVAL = atoi(argv[1]);
	if(TRANSMISSION_INTERVAL % 2 != 0){
		printf("warning: non-even transmission interval"); //currently only sleeping for Integer millis, division by 2 should give integers
	}
	//init wiringpi
	wiringPiSetup();
	pinMode(GPIO_IR_RECEIVER, INPUT);

	//init buffers
	current_receiving_byte = &received_bytes[0]; 

	//Wait until HIGH, then wait 3 timeslots
	printf("waiting...\n");
	//fflush(stdout);
	while(! digitalRead(GPIO_IR_RECEIVER));
	printf("starting with TRANSMISSION_INTERVAL=%d\n", TRANSMISSION_INTERVAL);
	delay(3*TRANSMISSION_INTERVAL);
	int done = 0;

	while(!done)
	{
		delay(TRANSMISSION_INTERVAL/2);
		unsigned char received_bit = (unsigned char) digitalRead(GPIO_IR_RECEIVER);
		done = receive_bit(received_bit);
		delay(TRANSMISSION_INTERVAL/2);
	}

	printf("done receiving!\n");
	printf("%s", (char*)received_bytes);

	fflush(stdout);
	sleep(1);
	return 0;
}
