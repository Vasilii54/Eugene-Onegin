// флаги для сортировки
// библиотека из своих строковых функций -> НАДО СДЕЛАТЬ АССЕРТЫ
// НА БАЗОВУЮ КАФЕДРУ ВЛИЯЮТ ЛЕТНИЕ СТАЖИРОВКИ
// НА СТАЖИРОВКИ МОЖНО ПОПАСТЬ ЕСЛИ ЗАКРЫВАТЬ ХУАВЕЙ И ОЧЕНЬ МНОГО РАБОТАТЬ И ТД
// МОЖНО ПОПАСТЬ В СБЕР И ТД,

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

#define N 100
#define NLINES 8000 // 7206


void Dump(const char* index[], int n);

void my_swap(void* a, void* b, size_t n);

void my_qsort(void* a, size_t n, size_t size,
              int(* comparator) (const void *, const void *));

int my_strcmp_up(const void* str1, const void* str2);

int my_strrcmp_up(const void* str1, const void* str2);

int my_strlen(const char *s);

ssize_t get_file_size(const char *path);

int fill_index(const char** index, char* buffer, int m);

void make_poem(const char** index, const char** new_poem, int num, int num_of_lines);

char* my_strchr(char* str, char* start, char x, int n);

void put_txt_to_file(const char** index, int n, FILE* file_out);

ssize_t do_input(char** buffer, char** buffer_original, const char* path);

int letter_key(unsigned char c);

int is_letter(unsigned char c);

// аргументы: 1. файл с текстом 2. файл вывода 3. кол-во новых строф 4. файл для новой поэмы

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Too few arguments!\n");
    }

    char* buffer = NULL;
    char* buffer_original = NULL;
    const char* index[NLINES] = {};

    ssize_t m = do_input(&buffer, &buffer_original, argv[1]);

    int n = fill_index(index, buffer, m);

    FILE* file_out = fopen(argv[2], "w");

    // Dump(index, n);

    my_qsort(index, n, sizeof(index[0]), my_strcmp_up);

    // Dump(index, n);

    fputs("��������������� ������� ������\n\n\n", file_out);

    put_txt_to_file(index, n, file_out);

    my_qsort(index, n, sizeof(index[0]), my_strrcmp_up);

    if(argc > 3) {
        int num = atoi(argv[3]); // ���������� �����
        const char** new_poem = NULL;
        new_poem = (const char**)calloc(num*5, sizeof(char*));
        make_poem(index, new_poem, num, n);
        Dump(new_poem, num*5);
        put_txt_to_file(new_poem, num*5, fopen(argv[4], "w"));
    }

    // Dump(index, n);

    fputs("\n\n========== !!!!!!!!!!!!!!! ==========================\n\n\n"
    "��������������� � ����� ������� ������\n\n\n", file_out);

    put_txt_to_file(index, n, file_out);

    fputs("\n\n========== !!!!!!!!!!!!!!! ==========================\n\n\n"
    "������������ ������� ������\n\n\n", file_out);

    fputs(buffer_original, file_out);

    fclose(file_out);
    free(buffer_original);
    free(buffer);

    return 0;
}

int letter_key(unsigned char c) {

    // ���������� �����
    if (c >= 'A' && c <= 'Z') {
        return 100 + (c - 'A');
    }

    if (c >= 'a' && c <= 'z') {
        return 100 + (c - 'a');
    }

    // �
    if (c == 0xA8 || c == 0xB8) {
        return 7;
    }

    // � � � � � �
    if (c >= 0xC0 && c <= 0xC5) {
        return c - 0xC0 + 1;
    }

    // � � � � � � � � � � � � � � � � � � � � � � � � � �
    if (c >= 0xC6 && c <= 0xDF) {
        return c - 0xC6 + 8;
    }

    // � � � � � �
    if (c >= 0xE0 && c <= 0xE5) {
        return c - 0xE0 + 1;
    }

    // � � � � � � � � � � � � � � � � � � � � � � � � � �
    if (c >= 0xE6 && c <= 0xFF) {
        return c - 0xE6 + 8;
    }

    return -1;
}


int is_letter(unsigned char c) {
    return letter_key(c) != -1;
}

ssize_t do_input(char** buffer, char** buffer_original, const char* path) {

    int size = get_file_size(path);
    // printf("File size is: %d\n", size);

    *buffer = (char*)calloc(size+1, sizeof(char));
    assert(buffer != NULL);

    int file;

    file = open(path, O_RDONLY);

    ssize_t m = read(file, *buffer, size);
    (*buffer)[m] = '\0';

    close(file);

    *buffer_original = (char*)calloc(m + 1, sizeof(char));
    assert(buffer_original != NULL);

    memcpy(*buffer_original, *buffer, m + 1);

    return m;
}

void put_txt_to_file(const char** index, int n, FILE* file_out) {
    for(int i = 0; i < n; ++i) {
        if(index[i][0] != '\0') {
            fputs(index[i], file_out);
            fputs("\n", file_out);
        }
    }
}

void make_poem(const char** index, const char** new_poem, int num, int num_of_lines) {
    /*
    �������� ��� ��������� ������ � ��� ������ ����� � ����,
     �������� 4 ������ ����� � ��� num ���
    */
    for(int i = 0; i < num*5; i += 5) {
        int num1 = rand() % (num_of_lines - 400);
        int num2 = rand() % (num_of_lines - 400);
        new_poem[i] = index[num1];
        new_poem[i+1] = index[num2];
        new_poem[i+2] = index[num1 + 1];
        new_poem[i+3] = index[num2 + 1];
        new_poem[i+4] = (char*)"\n";
    }
}

int fill_index(const char** index, char* buffer, int m) {
    int n = 0;

    if (m <= 0) {
        return 0;
    }

    // Первая строка начинается с начала буфера
    index[n++] = buffer;

    for (int i = 0; i < m; ++i) {
        if (buffer[i] == '\n') {
            buffer[i] = '\0';

            // Если после \n есть ещё символы, это начало следующей строки
            if (i + 1 < m) {
                index[n++] = buffer + i + 1;
            }
        }
    }

    return n;
}

char* my_strchr(char* str, char* start, char x, int n) {
    char* curr = start;
    while(curr < str + n) {
        if(*curr == x) {
            return curr;
        }
        ++curr;
    }
    return NULL;
}

void Dump(const char* index[], int n) {
    for(int i = 0; i < n; ++i) {
        printf("%d %s\n", i, index[i]);
    }
}

void my_qsort(void* a, size_t n, size_t size, int(* comparator) (const void *, const void *)) {
    if (n <= 1) return; // qsort
    // printf("It is beginning N = %d, array:\n", n);
    // print_arr(a, n);

    assert(a != NULL);

    size_t mid = n-1;
    size_t i = 0;

    while (i != mid) {
        int res = comparator((const void*) (a + i*size),(const void*) (a + mid*size));
        if (res >= 0) {
            my_swap(a+i*size,          a+mid*size-1*size, size);
            my_swap(a+mid*size-1*size, a+mid*size,        size);
            --mid;
            // printf("After swap:");
            // print_arr(a, n);
        } else {
            ++i;
            // printf("did ++i\n");
        }
    }
    // printf("It is end, a[mid] = %d N = %d, array:\n", a[mid], n);
    // print_arr(a, n);

    my_qsort(a,                       i, size, comparator);
    my_qsort(a+mid*size+1*size, n-mid-1, size, comparator);
}

void my_swap(void* a, void* b, size_t n) {
    unsigned char temp = '\0';
    for(int i = 0; i < n; ++i) {
        temp = *((unsigned char*)a+i);
        *((unsigned char*)a+i) = *((unsigned char*)b+i);
        *((unsigned char*)b+i) = temp;
    }

}

int my_strcmp_up(const void* str1, const void* str2) {
    const char* s1 = *(const char**)str1;
    const char* s2 = *(const char**)str2;

    while (*s1 && *s2) {

        if (!is_letter((unsigned char)*s1)) { ++s1; continue; }
        if (!is_letter((unsigned char)*s2)) { ++s2; continue; }
        int k1 = letter_key(*s1);
        int k2 = letter_key(*s2);
        if (k1 != k2) {
            return (k1 > k2) ? 1 : -1;
        }
        ++s1;
        ++s2;
    }

    // Пропускаем оставшиеся не-буквы в той строке, которая ещё не закончилась
    while (*s1 && !is_letter((unsigned char)*s1)) ++s1;
    while (*s2 && !is_letter((unsigned char)*s2)) ++s2;

    if (*s1 == '\0' && *s2 == '\0') return 0;
    return (*s1 == '\0') ? -1 : 1;
}
/*
int my_strrcmp_up(const void* string1, const void* string2) {
    const char* s1 = *(const char**)string1;
    const char* s2 = *(const char**)string2;

    size_t i = my_strlen(s1);
    size_t j = my_strlen(s2);

    while (1) {

        while (i > 0 &&
               !isalpha((unsigned char)s1[i - 1])) {
            --i;
        }

        while (j > 0 &&
               !isalpha((unsigned char)s2[j - 1])) {
            --j;
        }

        if (i == 0 || j == 0)
            break;

        unsigned char c1 =
            (unsigned char)tolower(
                (unsigned char)s1[i - 1]
            );

        unsigned char c2 =
            (unsigned char)tolower(
                (unsigned char)s2[j - 1]
            );

        if (c1 != c2) {
            return (c1 > c2) ? 1 : -1;
        }

        --i;
        --j;
    }

    if (i == 0 && j == 0)
        return 0;

    return (i == 0) ? -1 : 1;
}
*/

int my_strrcmp_up(const void* string1, const void* string2) {
    const char* str1 = *(const char**)string1;
    const char* str2 = *(const char**)string2;

    const char* s1 = NULL;
    s1 = str1 + my_strlen(str1) - 1;
    const char* s2 = NULL;
    s2 = str2 + my_strlen(str2) - 1;

    while ((s1 != str1-1) && (s2 != str2-1)) {

        if (!is_letter((unsigned char)*s1)) { --s1; continue; }
        if (!is_letter((unsigned char)*s2)) { --s2; continue; }
        int k1 = letter_key(*s1);
        int k2 = letter_key(*s2);
        if (k1 != k2) {
            return (k1 > k2) ? 1 : -1;
        }
        --s1;
        --s2;
    }

    // Пропускаем оставшиеся не-буквы в той строке, которая ещё не закончилась
    // while ((s1 != str1) && !isalpha((unsigned char)*s1)) --s1;
    // while ((s2 != str2) && !isalpha((unsigned char)*s2)) --s2;

    if ((s1 == str1-1) && (s2 == str2-1)) return 0;
    return (s1 == str1-1) ? -1 : 1;
}

int my_strlen(const char *s) {
    int i = 0;
    while(s[i] != '\0')
        i++;
    return i;
}

ssize_t get_file_size(const char *path) {
    struct stat st;

    if(stat(path, &st) == -1) { // возвращает -1 если файл не найден
        printf("file by path: %s didnt found\n", path);
        perror("stat"); // выводит сообщение
        return -1;
    }

    return (ssize_t)st.st_size; // приведение типа тк по умолчанию тип off_t
}
// см тип off_t
