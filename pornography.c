
#include <stdint.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep_ms(ms) Sleep(ms)
    #define LLU "%llu"
    #define LLD "%lld"
#else
    #define _POSIX_C_SOURCE 199309L
    #include <time.h>
    #include <locale.h>

    void sleep_ms(int ms) {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }

    static inline uint64_t max(uint64_t a, uint64_t b) {
        return a > b ? a : b;
    }

    #define LLU "%lu"
    #define LLD "%ld"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <limits.h>

#include "cJSON.h"

#define MAX_RETRIES 6

bool isValidInt(const char* str) {
    if (str == NULL || str[0] == '\0') return false;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] < 48 || str[i] > 57) return false;
        ++i;
    }
    return true;
}

uint64_t factorial(int x) {
    uint64_t ret = 1;
    for (int i = 1; i <= x; i++) ret *= i;
    return ret;
}

uint64_t calculatePermutationCount(const char* str) {
    if (str[0] == '\0') return -1; // fuck you
    int numeralsCount[10] = {0};
    int length = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        numeralsCount[str[i] - '0']++;
        length++;
    }
    uint64_t dividend = 1;
    for (int i = 0; i < 10; i++) dividend *= factorial(numeralsCount[i]);
    return factorial(length) / dividend;
}

// swap
void swap(char* a, char* b) {
    char temp = *a;
    *a = *b;
    *b = temp;
}

// merge sort stuff (code stolen from wikipedia im not doing this myself)

void copyArray(char a[], int begin, int end, char b[]) {
    for (int k = begin; k < end; ++k) {
        b[k] = a[k];
    }
}

void topDownMerge(char a[], int begin, int middle, int end, char b[]) {
    int i = begin;
    int j = middle;

    // Merge the two sorted runs into b
    for (int k = begin; k < end; ++k) {
        if (i < middle && (j >= end || a[i] <= a[j])) {
            b[k] = a[i]; // Take element from the left run
            i++;
        } else {
            b[k] = a[j]; // Take element from the right run
            j++;
        }
    }
}

void topDownSplitMerge(char a[], int begin, int end, char b[]) {
    if (end - begin <= 1) {
        return; // Base case: Run size is 1, so it's already sorted
    }

    int middle = (begin + end) / 2;  // Find the midpoint to split the array

    // Recursively sort the left and right halves into b
    topDownSplitMerge(b, begin, middle, a);
    topDownSplitMerge(b, middle, end, a);

    // Merge the sorted halves back into a
    topDownMerge(b, begin, middle, end, a);
}

void topDownMergeSort(char a[], char b[], int n) {
    // Copy the entire array a into b initially
    copyArray(a, 0, n, b);
    // Recursively split and merge the array b into a
    topDownSplitMerge(a, 0, n, b);
}

char* sort(const char* str) {
    uint64_t length = 0;
    while (str[length] != '\0') length++;
    char* base = (char*) calloc(length + 1, sizeof(char));
    char* b = (char*) calloc(length + 1, sizeof(char));

    memcpy(base, str, length + 1);

    topDownMergeSort(base, b, length);

    free(b);

    return base;
}

void addToPermutationList(const char* str, uint64_t* permsArray, uint64_t permsCount) {
    char* endptr;
    uint64_t strNum = strtoul(str, &endptr, 10);
    if (endptr == str) {
        fprintf(stderr, "No digits found in %s, somehow\n", str);
        exit(-69);
    }
    else if (*endptr != 0) {
        fprintf(stderr, "Non-numeric characters found in %s, somehow\n", str);
        exit(-67);
    }

    uint64_t firstEmpty = ULONG_MAX;
    for (uint64_t i = 0; i < permsCount; i++) {
        if (permsArray[i] == strNum) return;
        if (firstEmpty == ULONG_MAX && permsArray[i] == 0) firstEmpty = i;
    }
    if (firstEmpty != ULONG_MAX) permsArray[firstEmpty] = strNum;
}

void permutations(char* str, int len, uint64_t* permsArray, uint64_t permsCount) {
    if (len == 1) {
        addToPermutationList(str, permsArray, permsCount);
        return;
    }
    permutations(str, len - 1, permsArray, permsCount);
    for (int i = 0; i < len - 1; i++) {
        if (len % 2 == 0) swap(&str[i], &str[len - 1]);
        else swap(&str[0], &str[len - 1]);
        permutations(str, len - 1, permsArray, permsCount);
    }
}

void getPermutations(const char* str, uint64_t* permsArray, uint64_t permsCount) {
    char* base = sort(str);
    uint64_t length = 0;
    while (base[length] != '\0') length++;
    permutations(base, length, permsArray, permsCount);
    free(base);
}

typedef struct {
    char* data;
    size_t size;
} Data;

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t bytes = size * nmemb; // i hate this but this is the "canon" way to do it
    Data* data = (Data*)userdata;

    char* ndta = realloc(data->data, data->size + bytes + 1);

    if (!ndta) {
        fprintf(stderr, "realloc returned null, process does not have any available memory anymore, abandoning!\n");
        free(data->data);
        exit(420);
    }

    data->data = ndta;

    memcpy(&(data->data[data->size]), ptr, bytes);
    data->size += bytes;
    data->data[data->size] = 0;

    return bytes;
}



int main(int argc, char** argv) {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #else
        setlocale(LC_ALL, "");
    #endif
    if (argc != 2) {
        printf("Usage: %s [code]", argv[0]);
        return -1;
    }
    if (!isValidInt(argv[1])) {
        printf("Error: %s is not a valid input", argv[1]);
        return -1;
    }
    uint64_t permutationCount = calculatePermutationCount(argv[1]);
    printf("%s has " LLU " permutations\n", argv[1], permutationCount);

    uint64_t* perms = (uint64_t*) calloc(permutationCount, sizeof(uint64_t));

    if (perms == NULL) {
        fprintf(stderr, "issue with alloc-ing enough space for codes, quitting!\n");
        return 19;
    }

    getPermutations(argv[1], perms, permutationCount);

    // for (uint64_t i = 0; i < permutationCount; i++) printf("Found code: %6ld\n", perms[i]);

    cJSON_Hooks hooks = {
        .malloc_fn = malloc,
        .free_fn = free
    };

    cJSON_InitHooks(&hooks);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();

    #if true

    if (curl) {
        uint64_t retryCount = 0;
        uint64_t lastTriedCode = 0;
        for (uint64_t i = 0; i < permutationCount; i++) {

            char* uri = NULL;
            int length = snprintf(NULL, 0, "https://nhentai.net/api/v2/galleries/" LLU, perms[i]);
            length++;
            uri = calloc(length, sizeof(char));
            if (uri == NULL) {
                fprintf(stderr, "error while producing allocation, quitting!\n");
                return -676767;
            }
            snprintf(uri, length, "https://nhentai.net/api/v2/galleries/" LLU, perms[i]);

            Data data = {
                .data = NULL,
                .size = 0
            };
            
            curl_easy_setopt(curl, CURLOPT_URL, uri);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
            CURLcode result = curl_easy_perform(curl);
            if (result == CURLE_OK) {
                int64_t status = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
                if (status == 429) {
                    curl_off_t seconds;
                    curl_easy_getinfo(curl, CURLINFO_RETRY_AFTER, &seconds);
                    printf("nhentai api upset, retrying " LLU " after %" CURL_FORMAT_CURL_OFF_T " seconds...\n", perms[i], seconds);
                    sleep_ms(max(seconds * 1000, 1000));
                    i--; // known issue can't be fucked
                    free(data.data);
                    free(uri);
                    if (perms[i] != lastTriedCode) {
                        lastTriedCode = perms[i];
                        retryCount = 0;
                    }
                    retryCount++;
                    if (retryCount > MAX_RETRIES) {
                        fprintf(stderr, LLU " attempts to get the gallery at " LLU " have passed, nhentai has decided to stop cooperating, abandoning now!\n", retryCount, perms[i]);
                        exit(100);
                    }
                    continue;
                }
                if (status == 200) {
                    cJSON* jsonHell = cJSON_Parse(data.data);
                    if (jsonHell == NULL) {
                        fprintf(stderr, "what the fuck did the nhentai api just send us\n");
                        free(data.data);
                        free(uri);
                        continue;
                    }
                    cJSON* title = cJSON_GetObjectItem(jsonHell, "title");
                    char* english = cJSON_GetStringValue(cJSON_GetObjectItem(title, "english")); 
                    char* japanese = cJSON_GetStringValue(cJSON_GetObjectItem(title, "japanese"));
                    char* pretty = cJSON_GetStringValue(cJSON_GetObjectItem(title, "pretty"));

                    if (pretty != NULL) printf("code " LLU " corresponds to gallery %s\n", perms[i], pretty);
                    else if (english != NULL) printf("code " LLU " corresponds to gallery %s\n", perms[i], english);
                    else if (japanese != NULL) printf("code " LLU " corresponds to gallery %s (translate yourself)\n", perms[i], japanese);
                    else {
                        fprintf(stderr, "issue with getting gallery name for code " LLU "; continuing!\n", perms[i]);
                    }
                    cJSON_Delete(jsonHell);
                }
                else if (status == 404) printf("code " LLU " does not correspond to a valid gallery\n", perms[i]);
                else fprintf(stderr, "error: libcurl returned " LLD " on code " LLU "\n", status, perms[i]);
            }
            else fprintf(stderr, "error on " LLU ": %s\n", perms[i], curl_easy_strerror(result));

            free(data.data);
            free(uri);
            curl_easy_reset(curl);
        }

    }

    #else

    for (uint64_t i = 0; i < permutationCount; i++) {

        char* uri = NULL;
        int length = snprintf(NULL, 0, "https://nhentai.net/api/v2/galleries/" LLU, perms[i]);
        length++;
        uri = calloc(length , sizeof(char));
        if (uri == NULL) {
            fprintf(stderr, "error while producing allocation, quitting!\n");
            return -676767;
        }
        snprintf(uri, length, "https://nhentai.net/api/v2/galleries/" LLU, perms[i]);

        printf("code %lu points to the gallery to be obtained at %s\n", perms[i], uri);

        free(uri);
    }

    #endif

    free(perms);
    curl_easy_cleanup(curl);

    curl_global_cleanup();


    return 0;
}