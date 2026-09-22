#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>


#define N 100
#define NLINES 8000 // 7206


void Dump(const char* index[], int n);
void my_swap(void* a, void* b, size_t n);
void my_qsort(void* a, size_t n, size_t size, int(* comparator) (const void *, const void *));
int my_strcmp_up(const void* str1, const void* str2);
int my_strrcmp_up(const void* str1, const void* str2);
int my_strlen(const char *s);
int get_file_size(const char *path);
int fill_index(const char** index, char* buffer, int m);
char* my_strchr(char* str, char* start, char x, int n);

int main() {
    char pathname[] = "C:/Users/MSI/Desktop/MEOW/First/DOPY_DEDA/Dop_4_Onegin/Eugene Onegin.txt";
    int size = get_file_size(pathname);
    // printf("File size is: %d\n", size);
    char* buffer = (char*)calloc(size+1, sizeof(char));
    buffer[size] = '\0';

    int file;
    const char* index[NLINES] = {};
    file = open(pathname, O_RDONLY);
    ssize_t m = read(file, buffer, size);
    close(file);

    char* buffer_original = (char*)calloc(size+1, sizeof(char));
    strcpy(buffer_original, buffer);

    int n = 0;
    n = fill_index(index, buffer, m);
    printf("n = %d\n", n);
    printf("after fill_index\n");
    FILE* file_out = fopen("Sorted Eugene.txt", "w");

    // Dump(index, n);

    my_qsort(index, n, sizeof(const char *), my_strcmp_up);
    printf("after first qsort\n");

    // Dump(index, n);

    fputs("Sorted Eugene Onegin\n\n\n", file_out);

    for(int i = 0; i < n; ++i) {
        fputs(index[i], file_out);
        fputs("\n", file_out);
    }

    my_qsort(index, n, sizeof(const char *), my_strrcmp_up);

    // Dump(index, n);

    fputs("\n\n========== !!!!!!!!!!!!!!! ==========================\n\n\n"
    "Reversed sorted Eugene Onegin\n\n\n", file_out);

    for(int i = 0; i < n; ++i) {
        fputs(index[i], file_out);
        fputs("\n", file_out);
    }

    fputs("\n\n========== !!!!!!!!!!!!!!! ==========================\n\n\n"
    "Orinal Eugene Onegin\n\n\n", file_out);

    fputs(buffer_original, file_out);

    return 0;
}
/*
int fill_index(const char** index, char* buffer, ssize_t m) {
    char* start = NULL;
    start = my_strchr(buffer, buffer, '\n', m);
    int i = 0;
    index[i++] = buffer;

    while(start != NULL) {
        *start = '\0';
        index[i++] = start + 1;
        start = my_strchr(buffer, start + 1, '\n', m);
    }
    return i;
} */

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

        if (!isalpha((unsigned char)*s1)) { ++s1; continue; }
        if (!isalpha((unsigned char)*s2)) { ++s2; continue; }
        if (tolower(*s1) != tolower(*s2)) {
            return (tolower(*s1) > tolower(*s2)) ? 1 : -1;
        }
        ++s1;
        ++s2;
    }

    // Пропускаем оставшиеся не-буквы в той строке, которая ещё не закончилась
    while (*s1 && !isalpha((unsigned char)*s1)) ++s1;
    while (*s2 && !isalpha((unsigned char)*s2)) ++s2;

    if (*s1 == '\0' && *s2 == '\0') return 0;
    return (*s1 == '\0') ? -1 : 1;
}

int my_strrcmp_up(const void* string1, const void* string2) {
    const char* str1 = *(const char**)string1;
    const char* str2 = *(const char**)string2;

    const char* s1 = NULL;
    s1 = str1 + my_strlen(str1) - 1;
    const char* s2 = NULL;
    s2 = str2 + my_strlen(str2) - 1;

    while ((s1 != str1-1) && (s2 != str2-1)) {

        if (!isalpha((unsigned char)*s1)) { --s1; continue; }
        if (!isalpha((unsigned char)*s2)) { --s2; continue; }
        if (tolower(*s1) != tolower(*s2)) {
            return (tolower(*s1) > tolower(*s2)) ? 1 : -1;
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

int get_file_size(const char *path) {
    struct stat st;

    if(stat(path, &st) == -1) { // возвращает -1 если файл не найден
        perror("stat"); // выводит сообщение
        return -1;
    }

    return (int)st.st_size; // приведение типа тк по умолчанию тип off_t
}
