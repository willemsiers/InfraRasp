#include <wiringPi.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#define GPIO_LED 1
#define END_BYTE 168

void printChar(char* tag, char val)
{
#if 0
	printf("%s:\t",tag);
	int i = 0;
	while (i < 8) {
		if (val & 0b10000000){ printf("1");}
		else{ printf("0");}
		val <<= 1;
		i++;
	}
	printf("\n");
#endif
}

int main(int argc, char *argv[]){
	setbuf(stdout, NULL);
	if(argc <= 2){
		fprintf(stderr, "Missing (some) arguments: transmission interval (delay), input file name");
		return(1);
	}

	const int TRANSMISSION_INTERVAL = atoi(argv[1]);
	const char* filename_input = argv[2];

	//init wiringpi
	wiringPiSetup();
	pinMode(GPIO_LED, OUTPUT);
	digitalWrite(GPIO_LED, LOW);

	FILE* input_file;
	input_file = fopen(filename_input, "r");

	assert(input_file != NULL);
	unsigned int read_checksum = 0;

	unsigned char file_bytes[300*1024]; //300kb
	int total_read_file = 0;
	int read_as_int = -1;
	while( (read_as_int = fgetc(input_file)) != EOF)
	{
		unsigned char read_as_char = (unsigned char)read_as_int;
		file_bytes[total_read_file++] = read_as_char;
		read_checksum += read_as_char;
	}
	printf(" total_read_file:\t%d\n", total_read_file);
	//sleep(1);

	unsigned int sent_checksum = 0;
	int total_sent_bytes = 0;
	int total_sent_bits = 0;

	//Signal begin, and start writing
	digitalWrite(GPIO_LED, HIGH);
	delay(3*TRANSMISSION_INTERVAL);
	clock_t time_start = clock();
	while(total_sent_bytes < total_read_file)
	{
		unsigned char to_send_byte  = file_bytes[total_sent_bytes];
		int bits_sent_byte = 0;
		sent_checksum += to_send_byte;
		while(bits_sent_byte < 8)
		{
			unsigned char to_send_bit = (to_send_byte & 0b10000000) >> 7;

			printChar("send",to_send_bit);

			digitalWrite(GPIO_LED, to_send_bit ? HIGH : LOW);
			delay(TRANSMISSION_INTERVAL);

			//sleep(1);
			to_send_byte <<= 1;
			bits_sent_byte++;
			total_sent_bits++;
		}
		total_sent_bytes++;
	}
	clock_t time_end = clock();
	printf("done\n");

	//TODO: time doesn't work properly
	double time_elapsed = ((double)time_end - time_start) / CLOCKS_PER_SEC * 1000;
	printf("time=%f\n", time_elapsed);
	printf("bits sent=%d \n", total_sent_bits);
	float bits_per_second = total_sent_bits / time_elapsed;
	printf("bitrate=%f bps\n", bits_per_second);

	assert(total_sent_bits == total_sent_bytes*8);
	assert(total_sent_bytes == total_read_file);

	printf("checksum read:\n%u\nchecksum sent:\n%u\n", read_checksum, sent_checksum);
	digitalWrite(GPIO_LED, LOW);
	return 0;
}
