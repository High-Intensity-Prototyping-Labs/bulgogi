/**
 * @file rules.h
 * @brief Rule engine for bulgogi project validation.
 */

#ifndef BUL_RULES_H
#define BUL_RULES_H

// Standard C Libraries 
#include <stddef.h>
#include <sys/wait.h>

// External Dependencies 
#include "yaml.h"

// Project Headers 
#include "yaml_ext.h"

// Settings 
#define BUL_SIG_SYM_INF ".."

/** Rule ID */
typedef unsigned int bul_rule_id_t;

/** Rule symbol */
typedef char* bul_rule_sym_t;

/** Rule type */
typedef enum {
        /** Fundamental */
        BUL_RULE_FUNDAMENTAL,
        /** Relational */
        BUL_RULE_RELATIONAL,
} bul_rule_type_t;
#define BUL_RULE_N_TYPES 2

/** Rule Countability */
typedef enum {
        /** None (0) */
        BUL_RULE_CNT_NONE,
        /** One (1) */
        BUL_RULE_CNT_ONE,
        /** Infinity (∞) */
        BUL_RULE_CNT_INF,
} bul_rule_cnt_t;
#define BUL_RULE_N_CNTS 3

/** Rule Uniqueness */
typedef enum {
        /** Rule is unique */
        BUL_RULE_UNIQUE,
        /** Rule is shared */
        BUL_RULE_SHARED,
} bul_rule_uni_t;
#define BUL_RULE_N_UNIQUES 2

/** Rule signature (unique properties) */
typedef struct bul_rule_sig {
        /** Whether the rule is `BUL_RULE_FUNDAMENTAL` or `BUL_RULE_RELATIONAL` */
        bul_rule_type_t type;
        /** Whether the rule appears no, one or infinite times */
        bul_rule_cnt_t  countability;
        /** Whether the rule is shared or unique */
        bul_rule_uni_t  uniqueness;

} bul_rule_sig_s;

/** Pattern rule */
typedef struct bul_rule {
        bul_rule_id_t   id;
        bul_rule_sig_s  sig;
        bul_rule_sym_t  symbol;
} bul_rule_s;

/** Rule parser context */
typedef struct bul_rules {
        /** Number of rules stored */
        size_t size;
        /** Whether the rule context is fundamental or relational */
        bul_rule_type_t ctx;
        /** Access rule by ID. This is also where mem is alloc'd. */
        bul_rule_s *by_id;
        /** Access rule by sig */
        bul_rule_s ***by_sig;

} bul_rules_s;


/**
 * @brief Initializes the rule engine context.
 *
 * @return Initialized `bul_rules_s` context.
 */
bul_rules_s bul_rules_init(void);

/**
 * @brief Processes the next sequential YAML event from the `rules.yaml` file.
 *
 * @param[in] rules Rules context to use.
 * @param[in] event YAML event to process.
 */
void bul_rules_next_event(bul_rules_s *rules, yaml_event_t *event);

/**
 * @brief Processes a scalar event from the YAML file.
 *
 * ASSUMPTIONS:
 * 1. The event passed is of the YAML_SCALAR_EVENT type.
 *
 * @param[in] rules Rules context to use.
 * @param[in] event Scalar event to process.
 */
void bul_rules_process_scalar(bul_rules_s *rules, yaml_event_t *event);

void bul_rules_add(bul_rules_s *rules, bul_rule_sym_t symbol);

bul_rule_cnt_t bul_detect_cnt(bul_rule_sym_t symbol);

/**
 * @brief Frees the allocated rules context.
 *
 * @param[in] rules Rule context to free.
 */
void bul_rules_free(bul_rules_s *rules);

#endif // BUL_RULES_H

/**
 * rules[F/R][0/1/INF][U/S]
 */
