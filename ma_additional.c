/**
 * HANNA KALISZUK "MOORE AUTOMATA"
 *
 * Library contains helper functions for the implementation of the "ma.h" library, which simulates Moore automata.
 **/

#include "ma.h"
#include "ma_additional.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define FILL_THE_BLOCK 63
#define BITS_PER_BLOCK 64

// Represents a linked list of outgoing connections from the specified bit of the automaton.
typedef struct outgoing {
    moore_t* aut_getting_signals;
    size_t bit_getting_signals;
    struct outgoing* next;
} outgoing_t;

// Represents a single incoming connection to the specified bit of the automaton.
typedef struct incoming {
    moore_t* source_aut;
    size_t source_bit;
} incoming_t;

uint64_t create_bit_mask(size_t const num_bits) {
    uint64_t const mask = (1ULL << num_bits) - 1;
    return mask;
}

/*
 * Returns the value of the bit at index 'bit_index' in the block at index 'block_index' of the automaton 'source'.
 *
 * If the pointer to the automaton is NULL or 'bit_index' is out of range, it returns -1 and sets errno to EINVAL.
 */
int get_bit(uint64_t const* source, size_t const bit_index) {
    if (!source) {
        errno = EINVAL;
        return -1;
    }

    size_t const block_index = bit_index / BITS_PER_BLOCK;
    size_t const bit_offset = bit_index % BITS_PER_BLOCK;

    return (int)((source[block_index] >> bit_offset) & 1);
}

/*
 * Sets the bit at index 'bit_index' in the block at index 'block_index' in the 'array' to the value 'bit_value'.
 *
 * Returns -1 and sets errno to EINVAL if the pointer to the automaton is NULL or 'bit_index' is out of range.
 */
void set_bit(int const bit_value, uint64_t* const array, size_t const block_index, size_t const bit_index) {
    if (!array) {
        errno = EINVAL;
        return;
    }

    if (bit_value == 1) array[block_index] |= (1ULL << bit_index);
    else array[block_index] &= ~(1ULL << bit_index);
}

/*
 * Removes a connection from the outgoing list 'head' by deleting the element with aut_gettong_signals = 'a_in' and
 * bit_getting_signals = 'bit'.
 */
static void remove_from_the_outgoing_list(outgoing_t** head, moore_t const* const a_in, size_t const bit) {
    if (!*head) {
        errno = EINVAL;
        return;
    }

    outgoing_t* current = *head;
    outgoing_t* prev = NULL;

    while (current) {
        if (current->aut_getting_signals == a_in && current->bit_getting_signals == bit) {
            // element found
            if (prev) {
                // it is not the first element of the list
                prev->next = current->next;
            }
            else {
                // it is the first element of the list
                *head = current->next;
            }

            free(current);
            return;
        }

        prev = current;
        current = current->next;
    }
}

/*
 * Allocates memory for the parts of the automaton. If the automaton is not supposed to have input signals, it allocates
 * all parts of the structure except for 'a->input' and 'a->incoming_connections'.
 *
 * Returns false and sets errno to EINVAL if the pointer to the automaton is NULL / m == 0 / s == 0 or to ENOMEM if a
 * memory allocation error occurs. Otherwise, it returns true.
 */
bool allocate_automaton(moore_t* a, size_t const n, size_t const m, size_t const s) {
    if (!a || m == 0 || s == 0) {
        errno = EINVAL;
        return false;
    }

    size_t const output_blocks = (m + FILL_THE_BLOCK) / BITS_PER_BLOCK;
    size_t const input_blocks = (n + FILL_THE_BLOCK) / BITS_PER_BLOCK;
    size_t const state_blocks = (s + FILL_THE_BLOCK) / BITS_PER_BLOCK;

    if (n == 0) {
        a->input = NULL;
        a->incoming_connections = NULL;
    }

    if (n != 0) {
        a->input = (uint64_t*)calloc(input_blocks, sizeof(uint64_t));
        if (!a->input) {
            errno = ENOMEM;
            return false;
        }
    }

    a->output = (uint64_t*)calloc(output_blocks, sizeof(uint64_t));
    if (!a->output) {
        errno = ENOMEM;
        if (a->input) free(a->input);
        return false;
    }

    a->state = (uint64_t*)calloc(state_blocks, sizeof(uint64_t));
    if (!a->state) {
        errno = ENOMEM;
        if (a->input) free(a->input);
        free(a->output);
        return false;
    }

    if (n != 0) {
        a->incoming_connections = (incoming_t**)calloc(n, sizeof(incoming_t*));
        if (!a->incoming_connections) {
            errno = ENOMEM;
            free(a->input);
            free(a->output);
            free(a->state);
            return false;
        }

        for (size_t i = 0; i < n; i++) {
            a->incoming_connections[i] = NULL;
        }
    }

    a->outgoing_connections = (outgoing_t**)calloc(m, sizeof(outgoing_t*));
    if (!a->outgoing_connections) {
        errno = ENOMEM;
        if (a->input) free(a->input);
        free(a->output);
        free(a->state);
        free(a->incoming_connections);
        return false;
    }

    for (size_t i = 0; i < m; i++) {
        a->outgoing_connections[i] = NULL;
    }

    return true;
}

bool initialize_automaton(moore_t* a, size_t const n, size_t const m, size_t const s, transition_function_t const t,
                          output_function_t const y) {
    if (!a || !t || !y || m == 0 || s == 0) {
        errno = EINVAL;
        return false;
    }

    a->input_signals_num = n;
    a->output_signals_num = m;
    a->state_signals_num = s;
    a->transition_function = t;
    a->output_function = y;

    return true;
}

/*
 * Copies the state of the automaton to its output. Only for simple automata.
 */
void identity_function(uint64_t* output, uint64_t const* state, size_t m, size_t s) {
    if (!output || !state || s != m || m == 0) {
        errno = EINVAL;
        return;
    }

    const size_t blocks = (m + FILL_THE_BLOCK) / BITS_PER_BLOCK;
    memcpy(output, state, blocks * sizeof(uint64_t));

    if (m % BITS_PER_BLOCK != 0) { // mask the last block if needed
        uint64_t const mask = create_bit_mask(m % BITS_PER_BLOCK);
        output[blocks - 1] &= mask;
    }
}

/*
 * The function sets the input of the automaton based on its connections to other automata. It ignores connected bits.
 */
void get_input(moore_t* a) {
    if (!a) {
        errno = EINVAL;
        return;
    }

    size_t const input_signals = a->input_signals_num;

    for (size_t i = 0; i < input_signals; i++) {
        incoming_t const* const current = a->incoming_connections[i];

        if (current) {
            moore_t const* const source_automaton = current->source_aut;
            size_t const source_bit = current->source_bit;

            if (source_automaton) {
                int const bit_value = get_bit(source_automaton->output, source_bit);
                size_t const block = i / BITS_PER_BLOCK;
                size_t const bit_index = i % BITS_PER_BLOCK;
                set_bit(bit_value, a->input, block, bit_index);
            }
        }
    }

    size_t const bit_offset = input_signals % BITS_PER_BLOCK;
    if (bit_offset != 0) { // mask the last block if needed
        size_t const block = input_signals / BITS_PER_BLOCK;
        uint64_t const mask = create_bit_mask(bit_offset);
        a->input[block] &= mask;
    }
}

bool null_in_the_array(moore_t* a[], size_t const size) {
    for (size_t i = 0; i < size; i++) {
        if (a[i] == NULL) {
            return true;
        }
    }

    return false;
}

/*
 * Function calculates the new state of the automaton based on its input and current state using 'transition_function'.
 * Then it sets the new state of the automaton.
 */
void calculate_new_state(moore_t* a) {
    if (!a) {
        errno = EINVAL;
        return;
    }

    size_t const state_offset = a->state_signals_num % BITS_PER_BLOCK;
    size_t const output_offset = a->output_signals_num % BITS_PER_BLOCK;
    size_t const state_blocks = (a->state_signals_num + FILL_THE_BLOCK) / BITS_PER_BLOCK;
    size_t const output_blocks = (a->output_signals_num + FILL_THE_BLOCK) / BITS_PER_BLOCK;

    uint64_t* next_state = (uint64_t*)calloc(state_blocks, sizeof(uint64_t));
    if (next_state == NULL) {
        errno = ENOMEM;
        return;
    }

    a->transition_function(next_state, a->input, a->state, a->input_signals_num, a->state_signals_num);
    memcpy(a->state, next_state, state_blocks * sizeof(uint64_t));

    if (state_offset != 0) { // mask the last block if needed
        uint64_t const mask = create_bit_mask(state_offset);
        a->state[state_blocks - 1] &= mask;
    }

    a->output_function(a->output, a->state, a->output_signals_num, a->state_signals_num);

    if (output_offset != 0) { // mask the last block if needed
        uint64_t const mask = create_bit_mask(output_offset);
        a->output[output_blocks - 1] &= mask;
    }

    free(next_state);
}

/*
 * The function creates a new connection between the 'source_bit' of the automaton 'gives_signals' and the 'bit' of the
 * automaton 'gets_signals' by adding a pointer to 'gives_signals' to the list of incoming connections.
 */
void create_incoming_connection(moore_t* const gets_signals, size_t const bit, moore_t* const gives_signals,
                                size_t const source_bit) {
    if (!gets_signals || !gives_signals || bit >= gets_signals->input_signals_num || source_bit >=
        gives_signals->output_signals_num) {
        errno = EINVAL;
        return;
    }

    if (gets_signals->incoming_connections[bit] != NULL) {
        free(gets_signals->incoming_connections[bit]);
        outgoing_t* current = gives_signals->outgoing_connections[source_bit];
        remove_from_the_outgoing_list(&current, gets_signals, bit);
        gives_signals->outgoing_connections[source_bit] = current;
    }

    incoming_t* new_connection = (incoming_t*)malloc(sizeof(incoming_t));
    if (!new_connection) {
        errno = ENOMEM;
        return;
    }

    new_connection->source_aut = gives_signals;
    new_connection->source_bit = source_bit;
    gets_signals->incoming_connections[bit] = new_connection;
}

/*
 * The function creates a new connection between the 'source_bit' of the automaton 'gives_signals' and the 'bit' of the
 * automaton 'gets_signals' by adding an element to the outgoing connections list and setting it as the start of the
 * connections list for the 'source_bit'.
 */
void create_outgoing_connection(moore_t* const gives_signals, size_t const source_bit, moore_t* const gets_signals,
                                size_t const bit) {
    if (!gives_signals || !gets_signals || source_bit >= gives_signals->output_signals_num || bit >=
        gets_signals->input_signals_num) {
        errno = EINVAL;
        return;
    }

    bool existing_connection = false;
    outgoing_t *current = gives_signals->outgoing_connections[source_bit];

    while (current && !existing_connection) {
        if (current->aut_getting_signals == gets_signals && current->bit_getting_signals == bit) {
            existing_connection = true;
        }
        current = current->next;
    }

    if (!existing_connection) {
        outgoing_t* new_connection = (outgoing_t*)malloc(sizeof(outgoing_t));
        if (!new_connection) {
            errno = ENOMEM;
            return;
        }

        new_connection->aut_getting_signals = gets_signals;
        new_connection->bit_getting_signals = bit;
        new_connection->next = gives_signals->outgoing_connections[source_bit];
        gives_signals->outgoing_connections[source_bit] = new_connection;
    }
}

/*
 * The function removes the connections of the 'bit' from 'a_in' by removing this element from the 'outgoing_connections'
 * list of the automaton providing the signal and setting the 'incoming_connections[bit]' element to NULL.
 *
 * If such a connection does not exist, it does nothing. If the value of 'bit' is out of range or the pointer 'a_in'
 * is NULL, it sets errno to EINVAL.
 */
void remove_the_connection(moore_t* a_in, size_t const bit) {
    if (!a_in || bit >= a_in->input_signals_num) {
        errno = EINVAL;
        return;
    }

    if (!a_in->incoming_connections[bit] || !a_in->incoming_connections[bit]->source_aut) {
        return;
    }

    // what is the bit 'bit' connected to
    size_t const source_bit = a_in->incoming_connections[bit]->source_bit;
    moore_t const* const source_aut = a_in->incoming_connections[bit]->source_aut;

    // remove the connection from the list of outgoing connections
    outgoing_t* current_out = source_aut->outgoing_connections[source_bit];
    remove_from_the_outgoing_list(&current_out, a_in, bit);
    source_aut->outgoing_connections[source_bit] = current_out;

    free(a_in->incoming_connections[bit]);
    a_in->incoming_connections[bit] = NULL;
}

/*
 * The function removes all connections of the automaton 'a', both incoming and outgoing. It iterates through the
 * elements of the 'a->incoming_connections' array and calls remove_the_connection for each element. Then it iterates
 * through the elements of the 'a->outgoing_connections' array and for each element, it iterates through the list of
 * connections. For each connected element, it calls remove_the_connection.
 */
void clear_the_connections(moore_t* a) {
    if (!a) {
        errno = EINVAL;
        return;
    }

    size_t const input_signals = a->input_signals_num;
    size_t const output_signals = a->output_signals_num;

    // removing the incoming connections
    for (size_t i = 0; i < input_signals; i++) {
        remove_the_connection(a, i);
    }

    // removing the outgoing connections
    for (size_t i = 0; i < output_signals; i++) {
        outgoing_t* current = a->outgoing_connections[i];

        while (current) {
            outgoing_t* next = current->next;

            moore_t* getting_signals = current->aut_getting_signals;
            size_t const receiver = current->bit_getting_signals;

            if (getting_signals->incoming_connections[receiver]) free(getting_signals->incoming_connections[receiver]);
            getting_signals->incoming_connections[receiver] = NULL;

            free(current);
            current = next;
        }
        a->outgoing_connections[i] = NULL;
    }
}

void free_automaton(moore_t* a) {
    if (a) {
        if (a->input) free(a->input);
        if (a->output) free(a->output);
        if (a->state) free(a->state);
        if (a->incoming_connections) {
            for (size_t i = 0; i < a->input_signals_num; i++) {
                if (a->incoming_connections[i]) {
                    free(a->incoming_connections[i]);
                }
            }
            free(a->incoming_connections);
        }
        if (a->outgoing_connections) {
            for (size_t i = 0; i < a->output_signals_num; i++) {
                outgoing_t* current = a->outgoing_connections[i];
                while (current) {
                    outgoing_t* next = current->next;
                    free(current);
                    current = next;
                }
            }
            free(a->outgoing_connections);
        }
        free(a);
    }
}
