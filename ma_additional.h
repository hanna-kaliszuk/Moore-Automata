#ifndef MA_A_H
#define MA_A_H

#include <stdbool.h>
#include "ma.h"

typedef struct outgoing outgoing_t;
typedef struct incoming incoming_t;

typedef struct moore {
    size_t input_signals_num;
    size_t output_signals_num;
    size_t state_signals_num;

    uint64_t* state;
    uint64_t* input;
    uint64_t* output;

    transition_function_t transition_function;
    output_function_t output_function;

    outgoing_t **outgoing_connections; // Tablica list ze wskaźnikami na automaty przyjmujące bity od tego automatu
    incoming_t **incoming_connections; // tablica wskaznikow na automaty od ktorych przyjmujemy wejscie

} moore_t;

uint64_t create_bit_mask(size_t const num_bits);
int get_bit(uint64_t const* source, size_t const bit_index);
void set_bit(int const bit_value, uint64_t* const array, size_t const block_index, size_t const bit_index);
bool allocate_automaton(moore_t* a, size_t const n, size_t const m, size_t const s);
bool initialize_automaton(moore_t* a, size_t const n, size_t const m, size_t const s, transition_function_t const t,
                          output_function_t const y);
void identity_function(uint64_t* output, uint64_t const * state, size_t m, size_t s);
void get_input(moore_t* a);
bool null_in_the_array(moore_t *a[], size_t const size);
void calculate_new_state(moore_t* a);
void create_incoming_connection(moore_t* const gets_signals, size_t const bit, moore_t* const gives_signals,
                                size_t const source_bit);
void create_outgoing_connection(moore_t* const gives_signals, size_t const source_bit, moore_t* const gets_signals,
                                size_t const bit);
void remove_the_connection(moore_t* a_in, size_t const bit);
void clear_the_connections(moore_t* a);
void free_automaton(moore_t* a);

#endif //MA_A_H
