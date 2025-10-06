#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "types.h"
#include "rx02.h"
#include "efs.h"
#include "layout.h"

size_t get_size(const char* path)
{
	struct stat buf;
	if(stat(path, &buf) == -1) {
		perror("stat");
		exit(1);
	}

	return buf.st_size;
}

#define CHECKSUM8(x)	((u8) (x) + (u8) ((x) >> 8))

u16 checksum(u8* memory, size_t size)
{
	u16 sum = 0;
	u16* p = (u16*) memory;
	while(size -= 2) {
		sum += U16L(*(p++));
	}
	return sum;
}

u8 checksum8(u8* memory, size_t size)
{
	u8 sum = 0;
	u8* p = memory;
	while(size--) {
		sum += *(p++);
	}
	return sum;
}

#define	CHECK(x)	\
	if((x) != 1) { \
		printf("error: unexpected EOF (%u)\n", __LINE__); \
		return 0; \
	}

u16 load_binary(u8* memory, FILE* f)
{
	u16 len;
	u16 addr;
	u8 cksum;
	u16 end = 0;
	size_t bytes = 0;

	memset(memory, 0, 65536);
	while(1) {
		int c;
		while((c = fgetc(f)) != EOF) {
			if(c == 1) {
				break;
			}
		}
		if(c == 1) {
			c = fgetc(f);
		}
		if(c == EOF) {
			printf("error: unexpected EOF\n");
			return 0;
		} else if(c != 0) {
			printf("error: invalid signature! [%02x]\n", c);
			return 0;
		}

		CHECK(fread(&len, 2, 1, f));
		CHECK(fread(&addr, 2, 1, f));
		bytes += len;
		printf("%06o: %d bytes\n", addr, len - 6);
		if(end < (addr + len - 6)) {
			end = addr + len - 6;
		}

		if(len - 6 > 0) {
			CHECK(fread(&memory[addr], len - 6, 1, f));
		}
		CHECK(fread(&cksum, 1, 1, f));
		cksum += checksum8(&memory[addr], len - 6);
		cksum += CHECKSUM8(len) + CHECKSUM8(addr) + 1;
		if(cksum) {
			printf("invalid checksum: %03o\n", (unsigned int) cksum);
			return 0;
		}
		if(len == 6) {
			if(((addr & 1) == 0) && (addr != 0)) {
				printf("invalid transfer address %06o\n", addr);
				return 0;
			}
			break;
		}
	}
	return end;
}

void disk_read(void* userdata, void* data, unsigned int block)
{
	/* printf("[RD %u]\n", block); */
	void* floppy = userdata;
	get_block(data, floppy, block);
}

void disk_write(void* userdata, void* data, unsigned int block)
{
	/* printf("[WR %u]\n", block); */
	void* floppy = userdata;
	set_block(floppy, block, data);
}

int main(int argc, char** argv)
{
	const char* self = *argv;
	argc--;
	argv++;

	const char* bootloader_file = NULL;
	const char* monitor_file = NULL;
	const char* output_file = NULL;
	const char* diskroot_path = NULL;
	const char* layout_path = NULL;
	const char* disk_label = "ROMVOL";

	unsigned int vtoc_blocks = 4;

	EFS efs;

	for(; argc; argc--, argv++) {
		if(!strcmp("-b", *argv) && argc > 1) {
			bootloader_file = argv[1];
			argc--;
			argv++;
		} else if(!strcmp("-m", *argv) && argc > 1) {
			monitor_file = argv[1];
			argc--;
			argv++;
		} else if(!strcmp("-o", *argv) && argc > 1) {
			output_file = argv[1];
			argc--;
			argv++;
		} else if(!strcmp("-d", *argv) && argc > 1) {
			diskroot_path = argv[1];
			argc--;
			argv++;
		} else if(!strcmp("-c", *argv) && argc > 1) {
			layout_path = argv[1];
			argc--;
			argv++;
		} else if(!strcmp("-l", *argv) && argc > 1) {
			disk_label = argv[1];
			argc--;
			argv++;
		} else if(!strcmp("-v", *argv) && argc > 1) {
			vtoc_blocks = atoi(argv[1]);
			argc--;
			argv++;
		} else if(!strcmp("-h", *argv) || !strcmp("--help", *argv)) {
			printf("Usage: %s [OPTIONS]\n"
				"\n"
				"OPTIONS\n"
				"  -b boot.bin     Use bootloader from boot.bin\n"
				"  -m mon.bin      Use monitor from mon.bin\n"
				"  -o disk.rx2     Write disk image to disk.rx2\n"
				"  -d diskroot     Use diskroot as content for the disk\n"
				"  -c content.lst  Use content file for the disk content\n"
				"  -l label        Disk label\n"
				"  -v blocks       Blocks reserved for VTOC (1 blcok = 16 entries)\n"
				"  -h, --help      Show this help message\n", self);
			return 0;
		} else {
			printf("Unknown option: %s\n", *argv);
			return 1;
		}
	}

	if(!output_file) {
		printf("Error: no output file\n");
		return 1;
	}

	if(!bootloader_file) {
		printf("Error: no bootloader file\n");
		return 1;
	}

	if(!monitor_file) {
		printf("Error: no monitor file\n");
		return 1;
	}

	if(!diskroot_path && !layout_path) {
		printf("Error: no disk root directory or layout file\n");
		return 1;
	}

	/* temporary storage for disk image */
	u8* disk = (u8*) malloc(RX02_SIZE);
	u8* memory = (u8*) malloc(65536);

	/**********************************************************************/
	/* load bootloader into memory */
	printf("Loading bootloader into memory...\n");
	FILE* boot = fopen(bootloader_file, "rb");
	if(!boot) {
		printf("Failed to open bootloader file: %s\n", strerror(errno));
		return 1;
	}
	size_t bootsize = load_binary(memory, boot);
	fclose(boot);
	if(bootsize == 0) {
		printf("Failed to load bootloader\n");
		return 1;
	}

	/* compute checksum */
	u16 cksum = checksum(memory, 512);
	*((u16*) &memory[052]) = U16L(-cksum);
	printf("boot sector checksum: %06o\n", cksum);

	/* copy bootloader to disk */
	printf("bootloader size: %lu\n", bootsize);
	if(bootsize > 512) {
		printf("Error: bootloader is too big\n");
		return 1;
	}

	if(bootsize > 256) {
		u16 sectors = bootsize / 256;
		if(bootsize % 256) {
			sectors++;
		}
		printf("bootloader fits into %d sectors\n", sectors);
		for(u16 i = 0; i < sectors; i++) {
			u16 sector = i * 2 + 1;
			size_t start = get_offset(1, sector);
			printf("copying sector %d...\n", i);
			memcpy(disk + start, memory + 256 * i, 256);
		}
	} else {
		size_t start = get_offset(1, 1);
		printf("bootloader fits into a single sector\n");
		printf("copying sector %d...\n", 0);
		memcpy(disk + start, memory, 256);
	}

	/**********************************************************************/
	/* load bootloader into memory */
	printf("Loading monitor into memory...\n");
	FILE* mon = fopen(monitor_file, "rb");
	if(!mon) {
		printf("Failed to open monitor file: %s\n", strerror(errno));
		return 1;
	}
	size_t monsize = load_binary(memory, mon);
	fclose(mon);
	if(monsize == 0) {
		printf("Failed to load monitor\n");
		return 1;
	}

	/* compute checksum */
	u16 moncksum = checksum(memory, monsize);
	cksum += monsize / 2;
	cksum -= moncksum;
	*((u16*) (disk + get_offset(1, 1) + 052)) = U16L(-cksum);
	*((u16*) (disk + get_offset(1, 1) + 054)) = U16L(monsize / 2);
	*((u16*) (disk + get_offset(1, 1) + 056)) = U16L(-moncksum);
	printf("updated boot sector checksum: %06o\n", cksum);
	printf("monitor checksum: %06o\n", moncksum);

	/* copy monitor image to disk */
	u16 mon_sectors = (monsize + 255) / 256;
	printf("monitor size: %lu (%u sectors)\n", monsize, mon_sectors);
	for(u16 i = 0; i < mon_sectors; i++) {
		u16 sect = 4 + i;
		size_t off = get_sector(sect);
		memcpy(disk + off, memory + i * 256, 256);
	}

	/* create EFS filesystem */
	unsigned int efs_block = 6;
	if(4 + mon_sectors > (2 * efs_block)) {
		printf("monitor too big\n");
		return 1;
	}

	EFSInit(&efs, disk, disk_read, disk_write);
	EFSInitDisk(&efs, disk_label, 512512 / 512, vtoc_blocks, efs_block);

	if(diskroot_path) {
		/* copy everything from diskroot onto the disk */
		DIR* diskroot = opendir(diskroot_path);
		if(!diskroot) {
			printf("Failed to open disk root directory: %s\n", strerror(errno));
			return 1;
		}

		struct dirent* file;
		while((file = readdir(diskroot)) != NULL) {
			if(file->d_type != DT_REG) {
				continue;
			}

			char* path = malloc(strlen(diskroot_path) + strlen(file->d_name) + 2);
			sprintf(path, "%s/%s", diskroot_path, file->d_name);
			struct stat buf;
			if(stat(path, &buf) == -1) {
				printf("Failed to stat %s: %s\n", path, strerror(errno));
				return 1;
			}

			if(buf.st_size == 0) {
				printf("%s is empty, skipping\n", file->d_name);
				free(path);
				continue;
			}

			void* data = malloc(buf.st_size);
			if(!data) {
				printf("Failed to allocated %lu bytes\n", buf.st_size);
				return 1;
			}

			FILE* fd = fopen(path, "rb");
			if(!fd) {
				printf("Failed to open file %s: %s\n", path, strerror(errno));
				return 1;
			}

			if(fread(data, buf.st_size, 1, fd) != 1) {
				printf("Failed to read %s\n", path);
				return 1;
			}

			fclose(fd);

			EFSStoreSequential(&efs, file->d_name, buf.st_size, data, EFS_DSCB_DS_TYPE_EXEC);

			free(data);
			free(path);
		}

		closedir(diskroot);
	}

	if(layout_path) {
		LAYOUTProcess(&efs, layout_path);
	}

	unsigned int used_blocks = EFSGetUsedBlocks(&efs);
	unsigned int free_blocks = EFSGetFreeBlocks(&efs);
	printf("DISK SPACE: %u (%u BYTES) USED / %u (%u BYTES) FREE (%u%%)\n",
			used_blocks, used_blocks * 512,
			free_blocks, free_blocks * 512,
			(unsigned int) round(free_blocks * 100.0 / (512512 / 512)));

	FILE* out = fopen(output_file, "wb");
	if(!out) {
		printf("Failed to open output file: %s\n", strerror(errno));
		free(memory);
		free(disk);
		return 1;
	}

	if(fwrite(disk, RX02_SIZE, 1, out) != 1) {
		printf("Error: failed to write output file\n");
	}
	fclose(out);

	free(memory);
	free(disk);
	return 0;
}
