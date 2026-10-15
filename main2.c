
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>

#define STANZA_LINES 5

typedef int (*Comparator)(const void*, const void*);

/* Prototypes */

int do_input(const char* path,
             char** buffer,
             char** buffer_original,
             size_t* length);

int fill_index(const char** index,
               char* buffer,
               size_t length,
               size_t capacity);

int put_txt_to_file(const char** index, size_t n, FILE* file_out);

void my_swap(void* a, void* b, size_t n);

void my_qsort(void* a,
              size_t n,
              size_t size,
              Comparator comparator);

int my_strcmp_up(const void* str1, const void* str2);

int my_strrcmp_up(const void* str1, const void* str2);

size_t my_strlen(const char* s);

int letter_key(unsigned char c);

int is_letter(unsigned char c);

void make_poem(const char** index,
               const char** new_poem,
               int num,
               int num_of_lines);


/* ============================================================
                         MAIN
   ============================================================ */

int main(int argc, char* argv[]) {
    /*
        argv[1] - input file
        argv[2] - sorted output file
        argv[3] - number of new stanzas
        argv[4] - generated poem file

        Example:
        ./Onegin2.exe input_1251.txt test2.txt 7 new_p2.txt
    */

    if (argc != 5) {
        fprintf(stderr,
                "Usage: %s input.txt sorted.txt num_stanzas new_poem.txt\n",
                argv[0]);

        return EXIT_FAILURE;
    }

    /* Validate the number of stanzas. */

    errno = 0;

    char* endptr = NULL;
    long parsed_num = strtol(argv[3], &endptr, 10);

    if (errno == ERANGE ||
        endptr == argv[3] ||
        *endptr != '\0' ||
        parsed_num <= 0 ||
        parsed_num > INT_MAX / STANZA_LINES) {

        fprintf(stderr, "Invalid number of stanzas: %s\n", argv[3]);
        return EXIT_FAILURE;
    }

    int num = (int)parsed_num;

    char* buffer = NULL;
    char* buffer_original = NULL;
    size_t length = 0;

    /*
        Read the input before opening output files.
        This also prevents accidentally destroying the input
        if an output path equals the input path.
    */

    if (do_input(argv[1],
                 &buffer,
                 &buffer_original,
                 &length) != 0) {

        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    if (length == 0) {
        fprintf(stderr, "Input file is empty: %s\n", argv[1]);

        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    /*
        fill_index returns int, so ensure the number of bytes
        can be represented safely.
    */

    if (length >= (size_t)INT_MAX) {
        fprintf(stderr, "Input file is too large.\n");

        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    /*
        A text file cannot have more lines than length + 1.
        Allocate enough space for every possible line.
    */

    size_t capacity = length + 1;

    if (capacity > SIZE_MAX / sizeof(const char*)) {
        fprintf(stderr, "Input is too large for the index array.\n");

        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    const char** index = calloc(capacity, sizeof(*index));

    if (index == NULL) {
        perror("calloc index");

        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    int n = fill_index(index, buffer, length, capacity);

    if (n < 0) {
        fprintf(stderr, "Failed to build the line index.\n");

        free(index);
        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    if (n < 2) {
        fprintf(stderr,
                "At least two non-empty lines are required; got %d.\n",
                n);

        free(index);
        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    fprintf(stderr, "Read bytes: %zu\n", length);
    fprintf(stderr, "Non-empty lines: %d\n", n);

    srand((unsigned int)time(NULL));


    /* ========================================================
                    OPEN SORTED OUTPUT FILE
       ======================================================== */

    FILE* file_out = fopen(argv[2], "w");

    if (file_out == NULL) {
        perror(argv[2]);

        free(index);
        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    int write_error = 0;


    /* ========================================================
                    SORT BY BEGINNING
       ======================================================== */

    my_qsort(index, (size_t)n, sizeof(*index), my_strcmp_up);

    if (fputs("Sorted Eugene Onegin by beginning\n\n",
              file_out) == EOF ||
        put_txt_to_file(index, (size_t)n, file_out) != 0) {

        write_error = 1;
    }


    /* ========================================================
                    SORT BY ENDING
       ======================================================== */

    my_qsort(index, (size_t)n, sizeof(*index), my_strrcmp_up);

    if (fputs("\n\n"
              "========== REVERSE SORTED ==========\n\n",
              file_out) == EOF ||
        put_txt_to_file(index, (size_t)n, file_out) != 0) {

        write_error = 1;
    }


    /* ========================================================
                    ORIGINAL TEXT
       ======================================================== */

    if (fputs("\n\n"
              "========== ORIGINAL TEXT ==========\n\n",
              file_out) == EOF) {

        write_error = 1;
    }

    if (fputs(buffer_original, file_out) == EOF) {
        write_error = 1;
    }

    if (fclose(file_out) == EOF) {
        write_error = 1;
    }

    if (write_error) {
        fprintf(stderr, "Error while writing %s\n", argv[2]);

        free(index);
        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }


    /* ========================================================
                    GENERATE A NEW POEM
       ======================================================== */

    size_t poem_size = (size_t)num * STANZA_LINES;

    if (poem_size > SIZE_MAX / sizeof(const char*)) {
        fprintf(stderr, "Generated poem is too large.\n");

        free(index);
        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    const char** new_poem =
        calloc(poem_size, sizeof(*new_poem));

    if (new_poem == NULL) {
        perror("calloc new_poem");

        free(index);
        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    /*
        index is currently sorted by ending.
        make_poem chooses neighbouring lines from that array.
    */

    make_poem(index, new_poem, num, n);

    FILE* poem_out = fopen(argv[4], "w");

    if (poem_out == NULL) {
        perror(argv[4]);

        free(new_poem);
        free(index);
        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    int poem_write_error =
        put_txt_to_file(new_poem, poem_size, poem_out) != 0;

    if (fclose(poem_out) == EOF) {
        poem_write_error = 1;
    }

    if (poem_write_error) {
        fprintf(stderr, "Error while writing %s\n", argv[4]);

        free(new_poem);
        free(index);
        free(buffer);
        free(buffer_original);

        return EXIT_FAILURE;
    }

    fprintf(stderr, "Sorted text written to: %s\n", argv[2]);
    fprintf(stderr, "Generated poem written to: %s\n", argv[4]);

    free(new_poem);
    free(index);
    free(buffer);
    free(buffer_original);

    return EXIT_SUCCESS;
}


/* ============================================================
                    READ INPUT FILE
   ============================================================ */

int do_input(const char* path,
             char** buffer,
             char** buffer_original,
             size_t* length) {

    if (path == NULL ||
        buffer == NULL ||
        buffer_original == NULL ||
        length == NULL) {

        fprintf(stderr, "Invalid argument in do_input.\n");
        return -1;
    }

    *buffer = NULL;
    *buffer_original = NULL;
    *length = 0;

    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        perror(path);
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror("fseek");

        fclose(file);
        return -1;
    }

    long file_size = ftell(file);

    if (file_size < 0) {
        perror("ftell");

        fclose(file);
        return -1;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        perror("fseek");

        fclose(file);
        return -1;
    }

    size_t capacity = (size_t)file_size;

    if (capacity == SIZE_MAX) {
        fprintf(stderr, "Input file is too large.\n");

        fclose(file);
        return -1;
    }

    char* data = malloc(capacity + 1);

    if (data == NULL) {
        perror("malloc data");

        fclose(file);
        return -1;
    }

    size_t bytes_read = fread(data, 1, capacity, file);

    if (ferror(file)) {
        perror("fread");

        free(data);
        fclose(file);

        return -1;
    }

    if (fclose(file) == EOF) {
        perror("fclose input");

        free(data);
        return -1;
    }

    /*
        Normalize Windows CRLF line endings to LF.
        This prevents stray '\r' characters at the ends of lines.
    */

    size_t write_pos = 0;

    for (size_t read_pos = 0;
         read_pos < bytes_read;
         ++read_pos) {

        if (data[read_pos] == '\r' &&
            read_pos + 1 < bytes_read &&
            data[read_pos + 1] == '\n') {

            continue;
        }

        data[write_pos++] = data[read_pos];
    }

    data[write_pos] = '\0';

    char* original = malloc(write_pos + 1);

    if (original == NULL) {
        perror("malloc original");

        free(data);
        return -1;
    }

    memcpy(original, data, write_pos + 1);

    *buffer = data;
    *buffer_original = original;
    *length = write_pos;

    return 0;
}


/* ============================================================
                    BUILD LINE INDEX
   ============================================================ */

int fill_index(const char** index,
               char* buffer,
               size_t length,
               size_t capacity) {

    if (index == NULL || buffer == NULL || capacity == 0) {
        return -1;
    }

    size_t n = 0;
    size_t start = 0;

    /*
        Parse lines until the end of the buffer.
        Empty lines are not stored in index.
        The original text is preserved in buffer_original.
    */

    for (size_t i = 0; i <= length; ++i) {

        if (i < length && buffer[i] != '\n') {
            continue;
        }

        if (i < length) {
            buffer[i] = '\0';
        }

        if (i > start) {
            if (n >= capacity) {
                fprintf(stderr, "Index array capacity exceeded.\n");
                return -1;
            }

            index[n++] = buffer + start;
        }

        start = i + 1;
    }

    return (int)n;
}


/* ============================================================
                    WRITE LINES TO FILE
   ============================================================ */

int put_txt_to_file(const char** index, size_t n, FILE* file_out) {

    if (index == NULL || file_out == NULL) {
        fprintf(stderr, "Invalid argument in put_txt_to_file.\n");
        return -1;
    }

    for (size_t i = 0; i < n; ++i) {

        if (index[i] == NULL) {
            fprintf(stderr, "NULL pointer at index[%zu].\n", i);
            return -1;
        }

        if (index[i][0] == '\0') {
            continue;
        }

        if (fputs(index[i], file_out) == EOF) {
            return -1;
        }

        if (fputc('\n', file_out) == EOF) {
            return -1;
        }
    }

    return 0;
}


/* ============================================================
                    GENERATE NEW STANZAS
   ============================================================ */

void make_poem(const char** index,
               const char** new_poem,
               int num,
               int num_of_lines) {

    if (index == NULL ||
        new_poem == NULL ||
        num <= 0 ||
        num_of_lines < 2) {

        return;
    }

    /*
        Each stanza contains four selected lines and a separator.

        new_poem[i]     = first selected line
        new_poem[i + 1] = second selected line
        new_poem[i + 2] = neighbour of the first line
        new_poem[i + 3] = neighbour of the second line
        new_poem[i + 4] = blank line between stanzas

        The starting index must be <= num_of_lines - 2,
        because we also access index[start + 1].
    */

    for (int i = 0; i < num * STANZA_LINES; i += STANZA_LINES) {

        int limit = num_of_lines - 1;

        int num1 = rand() % limit;
        int num2 = rand() % limit;

        new_poem[i]     = index[num1];
        new_poem[i + 1] = index[num2];
        new_poem[i + 2] = index[num1 + 1];
        new_poem[i + 3] = index[num2 + 1];

        /*
            put_txt_to_file adds one newline after every entry.
            This additional newline creates a blank line.
        */

        new_poem[i + 4] = "\n";
    }
}


/* ============================================================
                    GENERIC SWAP
   ============================================================ */

void my_swap(void* a, void* b, size_t n) {

    if (a == NULL || b == NULL || n == 0 || a == b) {
        return;
    }

    unsigned char* p = a;
    unsigned char* q = b;

    for (size_t i = 0; i < n; ++i) {
        unsigned char temp = p[i];
        p[i] = q[i];
        q[i] = temp;
    }
}


/* ============================================================
                    CUSTOM QUICKSORT
   ============================================================ */

void my_qsort(void* a,
              size_t n,
              size_t size,
              Comparator comparator) {

    if (n < 2) {
        return;
    }

    if (a == NULL || size == 0 || comparator == NULL) {
        return;
    }

    unsigned char* base = a;

    /*
        Copy the pivot so that its value stays unchanged
        while elements are swapped.
    */

    unsigned char* pivot = malloc(size);

    if (pivot == NULL) {
        /*
            Allocation-free fallback: insertion sort.
            It is slower in the worst case, but still sorts correctly.
        */

        for (size_t i = 1; i < n; ++i) {

            size_t j = i;

            while (j > 0 &&
                   comparator(base + j * size,
                              base + (j - 1) * size) < 0) {

                my_swap(base + j * size,
                        base + (j - 1) * size,
                        size);

                --j;
            }
        }

        return;
    }

    memcpy(pivot, base + (n / 2) * size, size);

    /*
        Three-way partition:

        [0, lt)   elements less than pivot
        [lt, i)   elements equal to pivot
        [i, gt)   elements not processed yet
        [gt, n)   elements greater than pivot
    */

    size_t lt = 0;
    size_t i = 0;
    size_t gt = n;

    while (i < gt) {

        int result = comparator(base + i * size, pivot);

        if (result < 0) {

            my_swap(base + lt * size,
                    base + i * size,
                    size);

            ++lt;
            ++i;
        }
        else if (result > 0) {

            --gt;

            my_swap(base + i * size,
                    base + gt * size,
                    size);
        }
        else {
            ++i;
        }
    }

    free(pivot);

    /*
        Elements equal to the pivot are already in their final
        partition, so they do not need recursive sorting.
    */

    my_qsort(base, lt, size, comparator);

    my_qsort(base + gt * size,
             n - gt,
             size,
             comparator);
}


/* ============================================================
                    CP1251 LETTER ORDER
   ============================================================ */

int letter_key(unsigned char c) {

    /* Latin uppercase letters. */

    if (c >= 'A' && c <= 'Z') {
        return 100 + (c - 'A');
    }

    /* Latin lowercase letters. */

    if (c >= 'a' && c <= 'z') {
        return 100 + (c - 'a');
    }

    /* Russian Ё and ё in Windows-1251. */

    if (c == 0xA8 || c == 0xB8) {
        return 7;
    }

    /* Russian uppercase А-Е. */

    if (c >= 0xC0 && c <= 0xC5) {
        return c - 0xC0 + 1;
    }

    /* Russian uppercase Ж-Я. */

    if (c >= 0xC6 && c <= 0xDF) {
        return c - 0xC6 + 8;
    }

    /* Russian lowercase а-е. */

    if (c >= 0xE0 && c <= 0xE5) {
        return c - 0xE0 + 1;
    }

    /* Russian lowercase ж-я. */

    if (c >= 0xE6 && c <= 0xFF) {
        return c - 0xE6 + 8;
    }

    return -1;
}


int is_letter(unsigned char c) {
    return letter_key(c) != -1;
}


/* ============================================================
                    LENGTH OF A STRING
   ============================================================ */

size_t my_strlen(const char* s) {

    size_t i = 0;

    while (s[i] != '\0') {
        ++i;
    }

    return i;
}


/* ============================================================
                    SORT BY BEGINNING
   ============================================================ */

int my_strcmp_up(const void* str1, const void* str2) {

    const char* s1 = *(const char* const*)str1;
    const char* s2 = *(const char* const*)str2;

    for (;;) {

        /* Skip punctuation, spaces, and other non-letter symbols. */

        while (*s1 != '\0' &&
               !is_letter((unsigned char)*s1)) {
            ++s1;
        }

        while (*s2 != '\0' &&
               !is_letter((unsigned char)*s2)) {
            ++s2;
        }

        if (*s1 == '\0' || *s2 == '\0') {
            break;
        }

        int k1 = letter_key((unsigned char)*s1);
        int k2 = letter_key((unsigned char)*s2);

        if (k1 != k2) {
            return (k1 > k2) ? 1 : -1;
        }

        ++s1;
        ++s2;
    }

    if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    }

    return (*s1 == '\0') ? -1 : 1;
}


/* ============================================================
                    SORT BY ENDING
   ============================================================ */

int my_strrcmp_up(const void* string1, const void* string2) {

    const char* str1 = *(const char* const*)string1;
    const char* str2 = *(const char* const*)string2;

    size_t i = my_strlen(str1);
    size_t j = my_strlen(str2);

    for (;;) {

        /*
            Work with indexes instead of str1 - 1.
            Forming a pointer before the start of a string
            would be undefined behaviour.
        */

        while (i > 0 &&
               !is_letter((unsigned char)str1[i - 1])) {
            --i;
        }

        while (j > 0 &&
               !is_letter((unsigned char)str2[j - 1])) {
            --j;
        }

        if (i == 0 || j == 0) {
            break;
        }

        int k1 = letter_key((unsigned char)str1[i - 1]);
        int k2 = letter_key((unsigned char)str2[j - 1]);

        if (k1 != k2) {
            return (k1 > k2) ? 1 : -1;
        }

        --i;
        --j;
    }

    if (i == 0 && j == 0) {
        return 0;
    }

    return (i == 0) ? -1 : 1;
}
