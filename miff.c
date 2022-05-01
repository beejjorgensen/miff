/*
** Sample archive file format
**
** Sequential chunks
**
** Header:
**
**   File name: 256 bytes, trailing 0 padding
**   File size: 8 bytes, unsigned 64-bit value, big-endian
**
** Unpadded raw file data immediately follows header.
**
** Lots of TODOs in this one. Demonstration purposes only.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX_FILENAME_LEN 256
#define BLOCK_SIZE 4096  // for copying file data

#define UNIX // TODO make this some kind of proper config

#ifdef UNIX
#include <sys/stat.h>  // For getting file sizes
#endif

void usage_exit(void)
{
    fprintf(stderr, "usage: miff a|x|l file.miff file [file ...]\n");
    exit(1);
}

uint64_t get_file_size(const char *filename)
{
#ifdef UNIX
    struct stat sb;

    if (stat(filename, &sb) == -1) {
        perror("miff: stat");
        exit(3);
    }

    return sb.st_size;
#else
#error Port get_file_size() to your system
#endif
}

void pack_big_endian_u64(unsigned char *buf, uint64_t i)
{
    *buf++ = i>>56; *buf++ = i>>48;
    *buf++ = i>>40; *buf++ = i>>32;
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

uint64_t unpack_big_endian_u64(unsigned char *buf)
{
    return ((uint64_t)buf[0]<<56) |
       ((uint64_t)buf[1]<<48) |
       ((uint64_t)buf[2]<<40) |
       ((uint64_t)buf[3]<<32) |
       ((uint64_t)buf[4]<<24) |
       ((uint64_t)buf[5]<<16) |
       ((uint64_t)buf[6]<<8)  |
       buf[7];
}

void import_file(FILE *outf, char *filename)
{
    FILE *inf = fopen(filename, "rb");

    if (inf == NULL) {
        fprintf(stderr, "miff: error opening %s for reading\n", filename);
        exit(4);
    }

    char block[BLOCK_SIZE];
    size_t count;

    while ((count = fread(block, 1, BLOCK_SIZE, inf)) > 0) {
        fwrite(block, 1, count, outf);
        // TODO error check write
    }

    // TODO error check with ferror()

    // If you needed padding on the file data, here's where to add it
    
    fclose(inf);
}

void write_chunk(FILE *outf, char *filename)
{
    // Get file name
    // TODO don't silently truncate filenames
    char padded_filename[MAX_FILENAME_LEN + 1] = {0}; // +1 to leave room for NUL
    snprintf(padded_filename, MAX_FILENAME_LEN, "%s", filename);

    // Get file size
    uint64_t file_size = get_file_size(filename);
    unsigned char file_size_buf[8];
    pack_big_endian_u64(file_size_buf, file_size);

    // Write the name TODO error check
    fwrite(padded_filename, 1, MAX_FILENAME_LEN, outf);

    // Write the file size TODO error check
    fwrite(&file_size_buf, 1, 8, outf);

    // Write the file
    import_file(outf, filename);
}

void add_files(char *outfile, char *files[])
{
    FILE *outf = fopen(outfile, "wb");

    if (outf == NULL) {
        fprintf(stderr, "miff: error opening %s for writing\n", outfile);
        exit(2);
    }

    for (char **p = files; *p != NULL; p++) {
        write_chunk(outf, *p);
    }

    fclose(outf);
}

void list_files(char *infile)
{
    FILE *inf = fopen(infile, "rb");

    if (inf == NULL) {
        fprintf(stderr, "miff: error opening %s fo reading\n", infile);
        exit(2);
    }

    while (!feof(inf) && !ferror(inf)) {
        char file_name[MAX_FILENAME_LEN + 1] = {0}; // +1 to leave room for NUL
        size_t count;

        // Read the file_name
        count = fread(file_name, 1, MAX_FILENAME_LEN, inf);

        if (count == 0) continue; // EOF, probably

        // Read the file length
        unsigned char file_size_buf[8];
        fread(file_size_buf, 1, 8, inf);
        // TODO error check
        uint64_t file_size = unpack_big_endian_u64(file_size_buf);

        printf("%s: %" PRIu64 " bytes\n", file_name, file_size);

        // Jump to the next chunk
        fseek(inf, file_size, SEEK_CUR);
    }

    fclose(inf);
}

int main(int argc, char **argv)
{
    if (argc < 2) usage_exit();

    switch (argv[1][0]) {
        case 'a':
            if (argc < 4) usage_exit();
            add_files(argv[2], argv+3);
            break;
        case 'x':
            if (argc < 3) usage_exit();
            // TODO
            //extract_files(argv[2], argv+3);
            break;
        case 'l':
            if (argc != 3) usage_exit();
            list_files(argv[2]);
            break;
        default:
            usage_exit();
    }
    
}
