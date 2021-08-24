#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <uci.h>

#define CONFFILE "/etc/config/mosquitto_client"

struct Topic {
    char *topic;
    char *qos;
    struct Topic *next;
};

struct Configuration {
    char *address;
    char *port;
    char *username;
    char *password;
    char *use_tls;
};

void insert_topic_into_linked_list(char *qos, char *topic, struct Topic **head) {
    struct Topic *link = (struct Topic*) malloc(sizeof(struct Topic));
    link->topic = (char*)malloc(strlen(topic) + 1);
    strcpy(link->topic,topic);
    link->qos = malloc(strlen(qos) + 1);
    strcpy(link->qos,qos);

    link->next = *head; //point it to old first node
    *head = link;       //point first to new first node
}

int uci_read_config(struct Topic **head, struct Configuration *config)
{   
        char *topic=NULL;
        char *qos=NULL;
        int ret = 0;
        struct uci_element *i_iter = NULL;
        struct uci_package *package = NULL;
        struct uci_context *ctx = uci_alloc_context();

        if (uci_load(ctx, CONFFILE, &package) != UCI_OK) {
            syslog (LOG_ERR,"Failed to load uci.");
            ret = 2;
            goto cleanup;
        }

        uci_foreach_element(&package->sections, i_iter) {

            struct uci_section *section = uci_to_section(i_iter);

            if (strcmp(section->type, "mqtt_client") == 0) {
                
                if (uci_lookup_option_string(ctx, section, "adress")!= NULL){
                config->address = malloc(strlen((char*)uci_lookup_option_string(ctx, section, "adress")) + 1);
                strcpy(config->address,(char*)uci_lookup_option_string(ctx, section, "adress"));
                }
                
                if (uci_lookup_option_string(ctx, section, "local_port")!= NULL){
                config->port = malloc(strlen((char*)uci_lookup_option_string(ctx, section, "local_port")) + 1);
                strcpy(config->port,(char*)uci_lookup_option_string(ctx, section, "local_port"));
                }

                if (uci_lookup_option_string(ctx, section, "use_tls_ssl")!= NULL){
                config->use_tls = malloc(strlen((char*)uci_lookup_option_string(ctx, section, "use_tls_ssl")) + 1);
                strcpy(config->use_tls,(char*)uci_lookup_option_string(ctx, section, "use_tls_ssl"));
                }

                if (uci_lookup_option_string(ctx, section, "username")!= NULL){
                config->username = malloc(strlen((char*)uci_lookup_option_string(ctx, section, "username")) + 1);
                strcpy(config->username,(char*)uci_lookup_option_string(ctx, section, "username"));
                }

                if (uci_lookup_option_string(ctx, section, "password")!= NULL){
                config->username = malloc(strlen((char*)uci_lookup_option_string(ctx, section, "password")) + 1);
                strcpy(config->username,(char*)uci_lookup_option_string(ctx, section, "password"));
                }
            }
            if (strcmp(section->type, "topic") == 0) {
                char *topic =(char*)uci_lookup_option_string(ctx, section, "topic");
                char *qos = (char*)uci_lookup_option_string(ctx, section, "qos");
                insert_topic_into_linked_list(qos,topic, head);
            }
        }
cleanup:
        uci_free_context(ctx);
        return ret;
}