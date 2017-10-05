#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// usage: eleaf_decrypt [filename]
// based on https://github.com/TBXin/NFirmwareEditor

#define MAGICNUM 0x63B38
static int g_magicnum = 0x63B38;
static size_t imgsize;

int gen_function(off_t bytecount, int index) {
	return bytecount + g_magicnum + index - (bytecount / g_magicnum);
}


char* decrypt(char *encrypted, off_t bytecount) {
	char *decrypted = calloc( (size_t )bytecount, sizeof(char)); //yolo cast
	for(int i = 0; i < bytecount; i++) {
		decrypted[i] = (encrypted[i] ^ gen_function(bytecount, i) & 0xFF);
	}
	return decrypted;
}

char* load_fw_image(const char *path, off_t bytecount) {
	FILE *fw_file = fopen(path, "r");
	char *fw_image = calloc((size_t) bytecount, sizeof(char));
	if (!fw_file) {
		fprintf(stderr, "fuck\n");
	}
	
	fread(fw_image, sizeof(char), bytecount, fw_file);
	fclose(fw_file);
	return fw_image;
}

// header seems to be fixed 2 bytes
/*char* get_body(const char *fw_image, off_t bytecount)
{
	return &(fw_image[2]);	
}
*/

char * load_and_decrypt(const char *path, const char *outfile) {
	struct stat filestat;
	off_t filesize;
	stat(path, &filestat);
	filesize = filestat.st_size;
	imgsize = (size_t) filesize;
	printf("filename: %s, size: %ld \n", path, filesize);
	char *fw_image = load_fw_image(path, filesize);
	fprintf(stderr, "fw image loaded, address: %p\n", fw_image);
	//char *fw_body = get_body(fw_image, filesize);

	//char * decrypted = decrypt(fw_body, filesize - 2);
	char * decrypted = decrypt(fw_image, filesize);
	char * fw_image_decrypted = calloc((size_t) filesize, sizeof(char));
	//fw_image_decrypted[0] = fw_image[0];
	//fw_image_decrypted[1] = fw_image[1];

	//memcpy(&fw_image_decrypted[2], decrypted, filesize-2);
	memcpy(fw_image_decrypted, decrypted, filesize);

	FILE * decrypted_file = fopen(outfile, "w");
	fwrite(fw_image_decrypted, sizeof(char), filesize, decrypted_file);
	fclose(decrypted_file);

	free(fw_image);
	free(decrypted);
	return fw_image_decrypted;
}

int main(int argc, char **argv)
{
	if(argc > 2) {
		char * fw_image_decrypted = load_and_decrypt(argv[1], argv[2]);
		if(memmem(fw_image_decrypted, imgsize, "Joyetech APROM", 14)) {
		//if(1) {
			printf("Firmware successfully decrypted.\n");
			free(fw_image_decrypted);
			return 0;
		}
		else {
			printf("Couldn't find marker string. Decryption unsuccessful?\n");
			free(fw_image_decrypted);
			return 2;
		}
		fprintf(stderr, "what");
		free(fw_image_decrypted);
		return 3;
	}
	else {
		fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
	}

	return 1;
}


