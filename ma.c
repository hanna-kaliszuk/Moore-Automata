/**
 * HANNA KALISZUK "AUTOMATY MOORE'A"
 *
 * Implementacja dynamicznie ładowanej biblioteki symulującej automaty Moore'a. Automat jest reprezentowany przez
 * strukturę moore_t. Rozważane są jedynie automaty binarne, które mają input_signals_num jednobitowych sygnałów
 * wejściowych, output_signals_num jednobitowych sygnałów wyjściowych, a stan ma state_signals_num bitów.
 *
 * W każdym kroku działania automatu funkcja transition_function na podstawie wartości sygnałów wejściowych i aktualnego
 * stanu automatu wylicza nowy stan automatu. Funkcja output_function na podstawie stanu automatu wylicza wartości
 * wygnałów wyjściowych. Są to funkcje implementowane przez użytkownika.
 *
 * Ciągi bitów i wartości sygnałów przechowywane są w tablicy, której elementy są typu uint64_t, a w jendym elemencie
 * przechowywane są kolejne 64 bity. W przypadku, gdy ciąg bitów nie jest wielokrotnością 64, to bardziej znaczące bity
 * ostatniego elementu tablicy pozostają nieużywane.
 *
 * Stan wejścia automatu ustala się za pomocą funkcji ma_set_input lub przez podłączenie tego wejścia do wyjścia
 * automatu. Dopóki stan wejścia nie zostanie ustalony, jest nieustalony - nie znane są wartości elementów tablicy.
 * Również po odłączeniu wejścia automatu od wyjścia automatu stan tego wejścia jest nieustalony.
 *
 * Implementacja wykorzystuje dodatkową bibliotekę "ma_additional.h", która zawiera funkcje pomocnicze.
 **/

#include "ma_additional.h"
#include "ma.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define FILL_THE_BLOCK 63
#define BITS_PER_BLOCK 64

/*
 * The function creates a new Moore automaton with 'n' input signals, 'm' output signals, and 's' internal state bits,
 * a transition function 't', and an output function 'y'. It also sets the initial state of the automaton to 'q'.
 * Unused bits of the automaton are initialized to zero.
 *
 * It returns a pointer to the structure representing the automaton, or NULL if any of the parameters 'm' or 's' is 0,
 * 't', 'y', or 'q' is NULL, or a memory allocation error occurred. In such cases, it sets errno to EINVAL or ENOMEM,
 * respectively.
 */
moore_t* ma_create_full(size_t n, size_t m, size_t s, transition_function_t t,
                        output_function_t y, uint64_t const* q) {
    moore_t* a = (moore_t*)malloc(sizeof(moore_t));
    if (!a) {
        errno = ENOMEM;
        return NULL;
    }

    if (!initialize_automaton(a, n, m, s, t, y)) {
        free(a);
        return NULL;
    }

    if (!allocate_automaton(a, n, m, s)) {
        free(a);
        return NULL;
    }

    if (ma_set_state(a, q) == -1) {
        free_automaton(a);
        return NULL;
    }

    return a;
}

/*
 * The function creates a new Moore automaton with 'n' input signals, 'm' output signals, and 'm' internal state bits,
 * a transition function 't', and the identity function as the output function. Initially, the automaton's state
 * and its output are set to zero.
 *
 * It returns a pointer to the structure representing the automaton, or NULL if the parameter 'm' is 0, 't' is NULL,
 * or a memory allocation error occurred. In such cases, it sets errno to EINVAL or ENOMEM, respectively.
 */
moore_t* ma_create_simple(size_t n, size_t m, transition_function_t t) {
    moore_t* a = (moore_t*)malloc(sizeof(moore_t));
    if (!a) {
        errno = ENOMEM;
        return NULL;
    }

    if (!initialize_automaton(a, n, m, m, t, identity_function)) {
        free(a);
        return NULL;
    }

    if (!allocate_automaton(a, n, m, m)) {
        free(a);
        return NULL;
    }

    return a;
}

/*
 * The function deletes the specified automaton, first clearing its connections and freeing the memory it uses.
 * It does nothing if called with a NULL pointer.
 */
void ma_delete(moore_t* a) {
    if (a) {
        clear_the_connections(a);
        free_automaton(a);
    }
}

/*
 * The function connects the next 'num' input signals of the automaton 'a_in' to the output signals of the automaton
 * 'a_out', starting from the signals numbered 'in' and 'out'. If the input was previously connected to an output,
 * it is disconnected.
 *
 * The function returns 0 if the signals were successfully connected. If any pointer is NULL, the parameter 'num' is 0,
 * the specified range of input or output numbers is invalid, or a memory allocation error occurred, the function
 * returns -1 and sets errno to EINVAL or ENOMEM.
 */
int ma_connect(moore_t* a_in, size_t in, moore_t* a_out, size_t out, size_t num) {
    if (!a_in || !a_out || num == 0 || in + num > a_in->input_signals_num || out + num > a_out->output_signals_num) {
        errno = EINVAL;
        return -1;
    }

    for (size_t i = 0; i < num; i++) {
        if (a_in->incoming_connections[in + i]) remove_the_connection(a_in, in + i);
        create_incoming_connection(a_in, in + i, a_out, out + i);
        create_outgoing_connection(a_out, out + i, a_in, in + i);
    }

    return 0;
}

/*
 * The function disconnects the next 'num' input signals of the automaton 'a_in' from the output signals of the
 * automaton 'a_out', starting from the input numbered 'in'. If any input was not connected, it remains so.
 *
 * The function returns 0 if the signals were successfully disconnected. If any pointer is NULL, the parameter 'num' is
 * 0, or the specified range of input numbers is invalid, the function sets errno to EINVAL and returns -1.
 */
int ma_disconnect(moore_t* a_in, size_t in, size_t num) {
    if (!a_in || num == 0 || in + num > a_in->input_signals_num) {
        errno = EINVAL;
        return -1;
    }

    for (size_t i = 0; i < num; i++) {
        remove_the_connection(a_in, in + i);
    }

    return 0;
}

/*
 * The function sets the values of signals on the unconnected inputs of the automaton, ignoring in the 'input' sequence
 * the bits corresponding to connected inputs.
 *
 * It returns 0 if the signals were successfully set. If any pointer is NULL or the automaton has no inputs, it sets
 * errno to EINVAL and returns -1.
 */
int ma_set_input(moore_t* a, uint64_t const* input) {
    if (!a || !input || a->input_signals_num == 0) {
        errno = EINVAL;
        return -1;
    }

    size_t const input_signals = a->input_signals_num;

    for (size_t i = 0; i < input_signals; i++) {
        if (!a->incoming_connections[i]) {
            // if there is no previous connection
            size_t const block = i / BITS_PER_BLOCK;
            size_t const bit_index = i % BITS_PER_BLOCK;

            int const bit_value = get_bit(input, i);
            set_bit(bit_value, a->input, block, bit_index);
        }
    }

    return 0;
}

/*
 * Sets the state of the automaton 'a' to the state 'state'. It returns 0 if the operation was successful or -1 if any
 * of the pointers is NULL, setting errno to EINVAL.
 */
int ma_set_state(moore_t* a, uint64_t const* state) {
    if (!a || !state) {
        errno = EINVAL;
        return -1;
    }

    size_t const state_signals = a->state_signals_num;
    size_t const blocks = (state_signals + FILL_THE_BLOCK) / BITS_PER_BLOCK;

    memcpy(a->state, state, blocks * sizeof(uint64_t));

    // because the state has changed, the recalculation of the output is necessary
    a->output_function(a->output, a->state, a->output_signals_num, a->state_signals_num);

    return 0;
}

/*
 * The function returns a pointer to the bit sequence containing the values of the signals at the automaton's output
 * or NULL if the pointer to the automaton is NULL, setting errno to EINVAL.
 */
uint64_t const* ma_get_output(moore_t const* a) {
    if (!a) {
        errno = EINVAL;
        return NULL;
    }

    return a->output;
}

/*
 * Funkcja wykonuje jeden krok obliczeń podanych automatów z tablicy 'at[]'. Wszystkie automaty działają równolegle i
 * synchronicznie. Oznacza to, że wartości stanów i wyjść po wywołaniu funkcji zależą jedynie od wartości stanów, wejść
 * i wyjść przed wywołaniem funkcji.
 *
 * Przakazuje w wyniku 0 lub -1, jeżeli któryś ze wskaźników w tablicy ma wartość NULL, num == 0 albo wystąpił błąd
 * alokacji pamięci, odpowiednio ustawiając errno na EINVAL lub ENOMEM.
 */
/*
 * The function performs one computation step for the given automata in the array 'at[]'. All automata operate in
 * parallel and synchronously. This means that the values of states and outputs after the function call depend only on
 * the values of states, inputs, and outputs before the function call.
 *
 * It returns 0 or -1 if any pointer in the array is NULL, 'num' is 0, or a memory allocation error occurred, setting
 * errno to EINVAL or ENOMEM, respectively.
 */
int ma_step(moore_t* at[], size_t num) {
    if (!at || num == 0 || null_in_the_array(at, num)) {
        errno = EINVAL;
        return -1;
    }

    // set the inputs
    for (size_t i = 0; i < num; i++) {
        get_input(at[i]);
    }

    // calculate the new states
    for (size_t i = 0; i < num; i++) {
        calculate_new_state(at[i]);
    }

    return 0;
}
