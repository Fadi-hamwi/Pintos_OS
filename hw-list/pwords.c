/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2021 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

typedef struct{
  word_count_list_t* wc;
  char* filename;
} ThreadArgs;

void *count_words_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    count_words(args->wc, fopen(args->filename, "r"));
    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {
    // Create the empty data structure.
    word_count_list_t word_counts;
    init_words(&word_counts);

    if (argc <= 1) {
        // Process stdin in a single thread.
        count_words(&word_counts, stdin);
    } else {
        int nthreads = argc - 1;
        pthread_t threads[nthreads];

        for (int t = 0; t < nthreads; t++) {
          ThreadArgs* thread_args = (ThreadArgs *) malloc(sizeof(ThreadArgs));
          thread_args->wc = &word_counts;
          thread_args->filename = argv[t+1];
          int rc = pthread_create(&threads[t], NULL, count_words_thread, (void *)thread_args);
          if (rc) {
              printf("ERROR; return code from pthread_create() is %d\n", rc);
              exit(-1);
          }
        }

        // Wait for all threads to finish
        for (int t = 0; t < nthreads; t++) {
            int rc = pthread_join(threads[t], NULL);
            if (rc) {
                printf("ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }
        
    }

    // Output final result of all threads' work.
    wordcount_sort(&word_counts, less_count);
    fprint_words(&word_counts, stdout);

    return 0;
}
