/**
 * @file yaml_ext.c
 * @brief Custom extensions to the libyaml API.
 */

#include "yaml_ext.h"

// Standard C Libraries 
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

void yaml_print_event(yaml_event_t *event) {
        char* event_str = NULL;
        yaml_char_t* tag_str = NULL;
        yaml_char_t* value_str = NULL;
        yaml_char_t* anchor_str = NULL;
        size_t length = SIZE_MAX;

        assert(event);

        switch(event->type) {
        case YAML_NO_EVENT:
                event_str = "YAML_NO_EVENT";
                break;
        case YAML_STREAM_START_EVENT:
                event_str = "YAML_STREAM_START_EVENT";
                break;
        case YAML_STREAM_END_EVENT:
                event_str = "YAML_STREAM_END_EVENT";
                break;
        case YAML_DOCUMENT_START_EVENT:
                event_str = "YAML_DOCUMENT_START_EVENT";
                break;
        case YAML_DOCUMENT_END_EVENT:
                event_str = "YAML_DOCUMENT_END_EVENT";
                break;
        case YAML_ALIAS_EVENT:
                event_str = "YAML_ALIAS_EVENT";
                break;
        case YAML_SCALAR_EVENT:
                event_str = "YAML_SCALAR_EVENT";
                tag_str = event->data.scalar.tag;
                value_str = event->data.scalar.value;
                anchor_str = event->data.scalar.anchor;
                length = event->data.scalar.length;
                break;
        case YAML_SEQUENCE_START_EVENT:
                event_str = "YAML_SEQUENCE_START_EVENT";
                break;
        case YAML_SEQUENCE_END_EVENT:
                event_str = "YAML_SEQUENCE_END_EVENT";
                break;
        case YAML_MAPPING_START_EVENT:
                event_str = "YAML_MAPPING_START_EVENT";
                break;
        case YAML_MAPPING_END_EVENT:
                event_str = "YAML_MAPPING_END_EVENT";
                break;
        }

        printf("yaml_event_t event {\n");
        printf("\t.type = %s,\n", event_str);
        
        if(tag_str) {
                printf("\t.tag = %s,\n", tag_str);
        }

        if(value_str) {
                printf("\t.value_str = %s,\n", value_str);
        }

        if(anchor_str) {
                printf("\t.anchor_str = %s,\n", anchor_str);
        }

        if(length != SIZE_MAX) {
                printf("\t.length = %lu,\n", length);
        }

        printf("}\n");
}
