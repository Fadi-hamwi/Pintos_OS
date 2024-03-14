/*
 * Implementation of the word_count interface using Pintos lists.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_l.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t* wclist) { 
    list_init(wclist);
}

size_t len_words(word_count_list_t* wclist) {
    return list_size(wclist);
}

word_count_t* find_word(word_count_list_t* wclist, char* word) {
    struct list_elem *p = NULL;
    for(p = list_begin(wclist); p != list_end(wclist); p = list_next(p)){
        word_count_t* word_entry = list_entry(p, word_count_t, elem);
        if(strcmp(word, word_entry->word) == 0){
            return word_entry;
        }
    }
    return NULL;
}

word_count_t* add_word(word_count_list_t* wclist, char* word) {
    
    word_count_t *word_exists = find_word(wclist, word);
    if(word_exists != NULL){
        word_exists->count += 1;
        return word_exists;
    }
    
    word_count_t* new_entry = (word_count_t*)malloc(sizeof(word_count_t));
    new_entry->word = strdup(word);
    new_entry->count = 1;

    list_push_back(wclist, &(new_entry->elem));
    return new_entry;
}


#include <stdio.h>

void fprint_words(word_count_list_t* wclist, FILE* outfile) {
    struct list_elem *p = NULL;
    for(p = list_begin(wclist); p != list_end(wclist); p = list_next(p)){
        word_count_t* word_entry = list_entry(p, word_count_t, elem);
        fprintf(outfile, "%*d\t%s\n",8, word_entry->count, word_entry->word);
    }
}


static bool less_list(const struct list_elem* ewc1, const struct list_elem* ewc2, void* aux) {
    word_count_t* wc1 = list_entry(ewc1, word_count_t, elem);
    word_count_t* wc2 = list_entry(ewc2, word_count_t, elem);

    if(wc1->count == wc2->count){
        if(strcmp(wc1->word, wc2->word) <= 0) return true;
        return false;
    }
    return wc1->count <= wc2->count;
}

void wordcount_sort(word_count_list_t* wclist,
                    bool less(const word_count_t*, const word_count_t*)) {
  list_sort(wclist, less_list, less);
}
