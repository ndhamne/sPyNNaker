#include <stdbool.h>
#include <spin1_api.h>
#include <debug.h>
#include <delay_extension/delay_extension.h>
#include <synapse_expander/delay_sender.h>
#include "matrix_generator_common.h"

struct matrix_generator_stdp {
    uint32_t n_words_per_pp_row_header;
    uint32_t n_half_words_per_pp_synapse;
    uint32_t weight_half_word;
};

void *matrix_generator_stdp_initialize(address_t *region) {
    struct matrix_generator_stdp *params = (struct matrix_generator_stdp *)
        spin1_malloc(sizeof(struct matrix_generator_stdp));
    address_t params_sdram = *region;
    spin1_memcpy(params, params_sdram, sizeof(struct matrix_generator_stdp));
    params_sdram = &(params_sdram[sizeof(struct matrix_generator_stdp) >> 2]);

    *region = params_sdram;
    return params;
}

void matrix_generator_stdp_free(void *data) {
    sark_free(data);
}

#define SYNAPSE_DELAY_MASK 0xFF
#define STDP_PLASTIC_PLASTIC_SIZE 0
#define STDP_PLASTIC_PLASTIC_OFFSET 1

// These word offsets are based on the start of the fixed region itself
#define STDP_FIXED_FIXED_SIZE 0
#define STDP_FIXED_PLASTIC_SIZE 1
#define STDP_FIXED_PLASTIC_OFFSET 2


static uint16_t _build_fixed_plastic_half_word(
        uint16_t delay, uint32_t type,
        uint16_t post_index, uint32_t synapse_type_bits,
        uint32_t synapse_index_bits) {
    uint16_t synapse_index_mask = ((1 << synapse_index_bits) - 1);

    uint16_t wrd  = post_index & synapse_index_mask;
    wrd |= ((type & ((1 << synapse_type_bits) - 1)) << synapse_index_bits);
    wrd |= ((delay & SYNAPSE_DELAY_MASK) <<
            (synapse_index_bits + synapse_type_bits));
    return wrd;
}

void matrix_generator_stdp_write_row(
        void *data,
        address_t synaptic_matrix, address_t delayed_synaptic_matrix,
        uint32_t n_pre_neurons, uint32_t pre_neuron_index,
        uint32_t max_row_n_words, uint32_t max_delayed_row_n_words,
        uint32_t synapse_type_bits, uint32_t synapse_index_bits,
        uint32_t synapse_type, uint32_t n_synapses,
        uint16_t *indices, uint16_t *delays, uint16_t *weights,
        uint32_t max_stage) {
    struct matrix_generator_stdp *params =
        (struct matrix_generator_stdp *) data;

    // Row address for each possible delay stage (including no delay stage)
    address_t row_address[max_stage];
    uint16_t space_half_words[max_stage];
    uint32_t n_row_words = max_row_n_words + 3;
    uint32_t n_delay_row_words = max_delayed_row_n_words + 3;
    row_address[0] = NULL;
    space_half_words[0] = max_row_n_words * 2;
    if (synaptic_matrix != NULL) {
        row_address[0] = &(synaptic_matrix[pre_neuron_index * n_row_words]);
    }
    if (delayed_synaptic_matrix != NULL) {
        address_t delayed_address =
            &(delayed_synaptic_matrix[pre_neuron_index * n_delay_row_words]);
        uint32_t single_matrix_size = n_pre_neurons * n_delay_row_words;
        for (uint32_t i = 1; i < max_stage; i++) {
            row_address[i] = &(delayed_address[single_matrix_size * (i - 1)]);
            space_half_words[i] = max_delayed_row_n_words * 2;
        }
    } else {
        for (uint32_t i = 1; i < max_stage; i++) {
            row_address[i] = NULL;
            space_half_words[i] = 0;
        }
    }

    // Add the header half words (zero initialised) to each row
    for (uint32_t i = 0; i < max_stage; i++) {
        if (row_address[i] != NULL) {
            row_address[i][STDP_PLASTIC_PLASTIC_SIZE] =
                params->n_words_per_pp_row_header;
            for (uint32_t j = 0; j < params->n_words_per_pp_row_header; j++) {
                row_address[i][j + STDP_PLASTIC_PLASTIC_OFFSET] = 0;
            }
            space_half_words[i] -= params->n_words_per_pp_row_header * 2;
        }
    }

    // Get the plastic-plastic position at the start of each row and keep track
    // of the number of half-words per row (to allow padding later)
    uint16_t *pp_address[max_stage];
    uint16_t n_half_words_per_row[max_stage];
    for (uint32_t i = 0; i < max_stage; i++) {
        n_half_words_per_row[i] = 0;
        if (row_address[i] != NULL) {
            pp_address[i] = (uint16_t *) &(row_address[i][
                STDP_PLASTIC_PLASTIC_OFFSET +
                params->n_words_per_pp_row_header]);
        } else {
            pp_address[i] = NULL;
        }
    }

    // Write the plastic-plastic part of the row
    for (uint32_t synapse = 0; synapse < n_synapses; synapse++) {

        // Weight
        uint16_t weight = weights[synapse];

        // Delay (mostly to get the stage)
        struct delay_value delay = get_delay(delays[synapse], max_stage);
        if (delay.stage > 0) {
            delay_sender_send(pre_neuron_index, delay.stage - 1);
        }

        // Put the weight words in place
        if (pp_address[delay.stage] == NULL) {
            log_error("Delay stage %u has not been initialised", delay.stage);
            rt_error(RTE_SWERR);
        }
        if (space_half_words[delay.stage] <
                params->n_half_words_per_pp_synapse) {
            log_warning(
                "Row %u only has %u half words of %u free - not writing",
                delay.stage, space_half_words[delay.stage],
                params->n_half_words_per_pp_synapse);
            continue;
        }

        uint16_t *weight_words = pp_address[delay.stage];
        pp_address[delay.stage] =
            &(pp_address[delay.stage][params->n_half_words_per_pp_synapse]);
        for (uint32_t i = 0; i < params->n_half_words_per_pp_synapse; i++) {
            weight_words[i] = 0;
        }
        weight_words[params->weight_half_word] = weight;
        n_half_words_per_row[delay.stage] +=
            params->n_half_words_per_pp_synapse;
        space_half_words[delay.stage] -= params->n_half_words_per_pp_synapse;
    }

    // Add padding to any rows that are not word-aligned
    // and set the size in words
    for (uint32_t i = 0; i < max_stage; i++) {
        if (row_address[i] != NULL) {
            if (n_half_words_per_row[i] & 0x1) {
                pp_address[i][0] = 0;
                pp_address[i] = &(pp_address[i][1]);
                n_half_words_per_row[i] += 1;
            }
            row_address[i][STDP_PLASTIC_PLASTIC_SIZE] +=
                n_half_words_per_row[i] >> 1;
        }
    }

    // PP address is now fixed region address
    // Set the fixed-fixed size to 0 and point to the fixed-plastic region
    uint32_t *fixed_address[max_stage];
    uint16_t *fp_address[max_stage];
    for (uint32_t i = 0; i < max_stage; i++) {
        if (pp_address[i] != NULL) {
            fixed_address[i] = (uint32_t *) pp_address[i];
            fp_address[i] = (uint16_t *)
                &(fixed_address[i][STDP_FIXED_PLASTIC_OFFSET]);
            fixed_address[i][STDP_FIXED_FIXED_SIZE] = 0;
            fixed_address[i][STDP_FIXED_PLASTIC_SIZE] = 0;
        } else {
            fixed_address[i] = NULL;
            fp_address[i] = NULL;
        }
    }

    // Write the fixed-plastic part of the row
    for (uint32_t synapse = 0; synapse < n_synapses; synapse++) {

        // Post-neuron index
        uint32_t post_index = indices[synapse];

        struct delay_value delay = get_delay(delays[synapse], max_stage);

        // Build synaptic word
        uint16_t fp_half_word = _build_fixed_plastic_half_word(
            delay.delay, synapse_type, post_index, synapse_type_bits,
            synapse_index_bits);

        // Write the half-word
        fp_address[delay.stage][0] = fp_half_word;
        fp_address[delay.stage] = &(fp_address[delay.stage][1]);

        // Increment the size of the current row
        fixed_address[delay.stage][STDP_FIXED_PLASTIC_SIZE] += 1;
    }
}
