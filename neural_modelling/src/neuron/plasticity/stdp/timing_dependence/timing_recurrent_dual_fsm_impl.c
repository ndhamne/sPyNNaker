#include "timing_recurrent_dual_fsm_impl.h"

//---------------------------------------
// Globals
//---------------------------------------
// Exponential lookup-tables
uint16_t pre_exp_dist_lookup[STDP_FIXED_POINT_ONE];
uint16_t post_exp_dist_lookup[STDP_FIXED_POINT_ONE];

// Global plasticity parameter data
plasticity_trace_region_data_t plasticity_trace_region_data;

typedef struct {
    int32_t accumulator_depression_plus_one;
    int32_t accumulator_potentiation_minus_one;
    uint16_t pre_exp_dist_lookup[STDP_FIXED_POINT_ONE];
    uint16_t post_exp_dist_lookup[STDP_FIXED_POINT_ONE];
} dual_fsm_config_t;

//---------------------------------------
// Functions
//---------------------------------------
uint32_t *timing_initialise(address_t address) {
    log_info("timing_initialise: starting");
    log_info("\tRecurrent dual-FSM STDP rule");
    dual_fsm_config_t *config = (dual_fsm_config_t *) address;

    // Copy plasticity region data from address
    plasticity_trace_region_data.accumulator_depression_plus_one =
            config->accumulator_depression_plus_one;
    plasticity_trace_region_data.accumulator_potentiation_minus_one =
            config->accumulator_potentiation_minus_one;

    log_info("\tAccumulator depression=%d, Accumulator potentiation=%d",
            plasticity_trace_region_data.accumulator_depression_plus_one - 1,
            plasticity_trace_region_data.accumulator_potentiation_minus_one + 1);

    // Copy LUTs from following memory
    // **HACK** these aren't actually int16_t-based but this function will
    // still work fine
    (void) maths_copy_int16_lut(
            config->pre_exp_dist_lookup, STDP_FIXED_POINT_ONE,
            (int16_t *) pre_exp_dist_lookup);
    (void) maths_copy_int16_lut(
            config->post_exp_dist_lookup, STDP_FIXED_POINT_ONE,
            (int16_t *) post_exp_dist_lookup);

    log_info("timing_initialise: completed successfully");

    return (address_t) &config[1];
}
