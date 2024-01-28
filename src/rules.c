/**
 * @file rules.c
 * @brief Rule engine for bulgogi project validation.
 */

#include "rules.h"
#include "yaml.h"
#include <stdlib.h>
#include <string.h>

bul_rules_s bul_rules_init() {
        return (bul_rules_s) {
                .size = 0,
                .by_id = malloc(sizeof(bul_rule_s)),
                .by_sig = malloc(BUL_RULE_N_TYPES * BUL_RULE_N_CNTS * BUL_RULE_N_UNIQUES * sizeof(bul_rule_s*)),
        };
}

void bul_rules_next_event(bul_rules_s *rules, yaml_event_t *event) {
        switch(event->type) {
        case YAML_SEQUENCE_START_EVENT:
                rules->ctx = BUL_RULE_RELATIONAL;
                break;
        case YAML_SEQUENCE_END_EVENT:
                rules->ctx = BUL_RULE_FUNDAMENTAL;
                break;
        case YAML_SCALAR_EVENT:
                bul_rules_process_scalar(rules, event);
                break;
        default:
                break;
        }
}

void bul_rules_process_scalar(bul_rules_s *rules, yaml_event_t *event) {
        
}

void bul_rules_add(bul_rules_s *rules, bul_rule_sym_t symbol) {
        bul_rule_cnt_t cnt = BUL_RULE_CNT_ONE;

        // Grow rules // 
        
        cnt = bul_detect_cnt(symbol);

        bul_rule_s rule = {
                .sig = {
                        .type = rules->ctx,
                        .countability = cnt,
                        .uniqueness = BUL_RULE_UNIQUE,
                },
        };
}

bul_rule_cnt_t bul_detect_cnt(bul_rule_sym_t symbol) {
        bul_rule_cnt_t cnt = BUL_RULE_CNT_ONE;

        size_t sym_len = 0;
        size_t sig_len = 0;

        sym_len = strlen(symbol);
        sig_len = strlen(BUL_SIG_SYM_INF);

        if(sym_len > sig_len) {
                if(strncmp(&symbol[sym_len-2], BUL_SIG_SYM_INF, sig_len) == 0) {
                        cnt = BUL_RULE_CNT_INF;
                }
        }
        /* Otherwise, defaults to ONE */

        return cnt;
}

void bul_rules_free(bul_rules_s *rules) {
        free(rules->by_id);
        free(rules->by_sig);
}
