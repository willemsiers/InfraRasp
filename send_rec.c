#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h> //memset
#define DATA_LENGTH 1

void printChar(char* tag, char val)
{
#if 0
	printf("%s:\t",tag);
	int i = 0;
	while (i < 8) {
		if (val & 0b10000000){
			printf("1");}
		else{
			printf("0");}
		val <<= 1;
		i++;
	}
	printf("\n");
#endif
}

unsigned char received_bytes[300*1024];
unsigned int received_bits_total = 0;
unsigned int received_bytes_total = 0;
unsigned int received_checksum = 0;

unsigned char* current_receiving_byte = NULL;

void 
receive_init()
{
	current_receiving_byte = &received_bytes[0]; 
}

void
done_receive_byte()
{
	received_checksum += *current_receiving_byte;
	//printf("receive checksum %u\n", received_checksum);

	current_receiving_byte = &received_bytes[ ++ received_bytes_total]; 
	//memset(current_receiving_packet->data, 0, DATA_LENGTH);
}

	void
receive_bit(unsigned char bit)
{
	unsigned char bit_in_place = bit << (7 - (received_bits_total % 8));
	printChar("rec",bit_in_place);
	(*current_receiving_byte) |= bit_in_place;

	received_bits_total++;
	if(received_bits_total % 8 == 0)
	{
		done_receive_byte();
	}
}

int main(int argc, char *argv[]){
	FILE* input_file;
	printf("argc:%d\n",argc);
	if(argc > 1) {
		input_file = fopen(argv[1], "r");
	}else{
		input_file = fopen("send.c", "r");
	}
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

	receive_init();
	unsigned int sent_checksum = 0;
	int total_sent_bytes = 0;
	int total_sent_bits = 0;
	while(total_sent_bytes < total_read_file)
	{
		unsigned char to_send_byte  = file_bytes[total_sent_bytes];
		int bits_sent_byte = 0;
		sent_checksum += to_send_byte;
		while(bits_sent_byte < 8)
		{
			unsigned char to_send_bit = (to_send_byte & 0b10000000) >> 7;

			printChar("send",to_send_bit);
			receive_bit(to_send_bit);
			//sleep(1);
			to_send_byte <<= 1;
			bits_sent_byte++;
			total_sent_bits++;
		}
		total_sent_bytes++;
	}


	assert(total_sent_bits == total_sent_bytes*8);
	assert(total_sent_bytes == total_read_file);

	printf("checksum read:\n%u\nchecksum sent:\n%u\n", read_checksum, sent_checksum);
	printf("receive checksum %u\n", received_checksum);
	printf("TOTAL BYTES:\ntotal read:\n%u\ntotal sent:\n%u\ntotal received:\n%u\n", total_read_file, total_sent_bytes, received_bytes_total);
	return 0;
}
