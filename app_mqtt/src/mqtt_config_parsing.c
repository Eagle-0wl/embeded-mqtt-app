#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

#define CONFFILE "/etc/config/mosquitto_client"

struct Topic {
    char *topic;
    char * qos;
    struct Topic *next;
};

struct Configuration {
    char *address;
    char *port;
    char *username;
    char *password;
    char *use_tls;
};

void insert_data_to_linked_list(char *qos, char *combine, struct Topic **head) {
   struct Topic *link = (struct Topic*) malloc(sizeof(struct Topic));
   link->topic = combine;
   link->qos = qos;
   link->next = *head; //point it to old first node
   *head = link;       //point first to new first node
}

char *parse_data_from_config(char *temp_string){
    size_t needed;
    char combine[256];
    char* returnvalue=NULL;
    int quotes_counter=0;
    int iterator=0;
    memset(combine,'\0',256);
        for (int i=0;i <strlen(temp_string); i++){
            if (temp_string[i] == '\''){
                iterator=0;
                quotes_counter++;
            }
            if (temp_string[i] != '\'' && quotes_counter == 1 ){
                combine[iterator]=temp_string[i];
                iterator++;
            }else if (temp_string[i] == '\'' && quotes_counter == 2){
                quotes_counter=0;
                needed = snprintf(NULL, 0, "%s",combine) + 1;
                returnvalue = (char*) malloc(needed);
                sprintf(returnvalue, "%s",combine);
                memset(combine,'\0',256);
                iterator=0;
                break;
            }
        }
    return returnvalue;
}

int read_config_file(struct Topic **head, struct Configuration *config)
{  
    char *topic=NULL;
    char *qos=NULL;
    char temp_string[256];
    FILE *file;    
    file=fopen(CONFFILE,"r");               //opening file for read operation

    if (file == NULL){    //check if file opend
            syslog(LOG_ERR, "Could not open file");
            return -1;
        } 
    //creating and initializing temporary string used for reading file
    memset(temp_string, 0, sizeof temp_string);
    while ((fgets(temp_string, 256, file)) != NULL) {
        if (strlen(temp_string)!=0 && strlen(temp_string)!=1) {
            if (strstr(temp_string,"config topic")){
                memset(temp_string, 0, sizeof temp_string); 
                fgets(temp_string, 256, file);
                if (strstr (temp_string,"option topic")){
                    topic = parse_data_from_config(temp_string);
                    fgets(temp_string, 256, file);
                    qos = parse_data_from_config(temp_string); 
                    insert_data_to_linked_list(qos,topic, head);
                }else{
                    qos = parse_data_from_config(temp_string);
                    fgets(temp_string, 256, file);           
                    topic = parse_data_from_config(temp_string);
                    insert_data_to_linked_list(qos,topic, head);
                }
            }
            if (strstr (temp_string,"option adress"))
                config->address = parse_data_from_config(temp_string);
            else if (strstr (temp_string,"option local_port"))
                config->port = parse_data_from_config(temp_string);
            else if (strstr (temp_string,"option use_tls_ssl"))
                config->use_tls = parse_data_from_config(temp_string);
            else if (strstr (temp_string,"option username"))
                config->username = parse_data_from_config(temp_string);
            else if (strstr (temp_string,"option password"))
                config->password = parse_data_from_config(temp_string);
        }
        memset(temp_string, 0, sizeof temp_string);         //clearing temporary string*/ 
    }
    fclose(file);
    return 0;
}