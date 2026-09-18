#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define N 100
#define NLINES 7206

void Dump(const char* index[], int n);
void my_swap(void* a, void* b, size_t n);
void my_qsort(void* a, size_t n, size_t size, int(* comparator) (const void *, const void *));
int my_strcmp_up(const void* str1, const void* str2);
int my_strrcmp_up(const void* str1, const void* str2);
int my_strlen(const char *s);

int main() {
    FILE* file;
    char buf[N];
    const char* index[NLINES] = {};
    const char* index_copy[NLINES] = {};
    file = fopen("Eugene Onegin.txt", "r");
    int n = 0;
    while(n < NLINES && fgets(buf, N, file) != NULL) {
        // printf("%s", arr);
        index[n] = strdup(buf);
        index_copy[n] = strdup(buf);
        assert(index[n] != NULL);
        ++n;
    }
    fclose(file);

    FILE* file_out = fopen("Sorted Eugene.txt", "w");

    // Dump(index, n);

    my_qsort(index, n, sizeof(const char *), my_strcmp_up);

    // Dump(index, n);

    fputs("Sorted Eugene Onegin\n\n\n", file_out);
    for(int i = 0; i < n; ++i) {
        fputs(index[i], file_out);
    }

    my_qsort(index, n, sizeof(const char *), my_strrcmp_up);

    // Dump(index, n);


    fputs("\n\n========== !!!!!!!!!!!!!!! ==========================\n\n\n"
    "Reversed sorted Eugene Onegin\n\n\n", file_out);

    for(int i = 0; i < n; ++i) {
        fputs(index[i], file_out);
    }

    fputs("\n\n========== !!!!!!!!!!!!!!! ==========================\n\n\n"
    "Orinal Eugene Onegin\n\n\n", file_out);

    for(int i = 0; i < n; ++i) {
        fputs(index_copy[i], file_out);
    }
    return 0;
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
    char temp = '\0';
    for(int i = 0; i < n; ++i) {
        temp = *((char*)a+i);
        *((char*)a+i) = *((char*)b+i);
        *((char*)b+i) = temp;
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
