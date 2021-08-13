#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <uci.h>
#include <syslog.h>

volatile int interrupt = 0;

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
    char *enable;
    char *use_tls;
    char *tls_type;
    char *tls_version;
    char *preshared_key;
    char *identity;
    char *client_enabled;
};

// struct Topic *head = NULL;

void printList(struct Topic **head) {
   struct Topic *ptr = *head;
   //start from the beginning
   
   while(ptr != NULL) {
      printf("%s \n",ptr->topic);
      printf("%s \n",ptr->qos);
      ptr = ptr->next;
   }
}

void insertFirst(char *qos, char *combine, struct Topic ***head) {
   //create a link
   size_t needed;
   struct Topic *link = (struct Topic*) malloc(sizeof(struct Topic));

   link->topic = (char*) malloc(needed);
   link->topic = combine;
   link->qos = qos;
   //point it to old first node
   link->next = **head;
	
   //point first to new first node
   **head = link;
}

void sigHandler(int signo) {
    signal(SIGINT, NULL);
    syslog(LOG_INFO, "Received signal: %d", signo);
    closelog();
    interrupt = 1;
}

char * parse_config_topic(char *temp_string){
    size_t needed;
    char combine[256];
    char* returnvalue=NULL;
    int quotes_counter=0;
    int iterator=0;
    memset(combine,'\0',256);
    printf ("%s \n",temp_string);
        for (int i=0;i <strlen(temp_string); i++){
            if (temp_string[i] == '\''){
                iterator=0;
                quotes_counter++;
            }
            if (temp_string[i] != '\'' && quotes_counter == 1 ){
                //printf ("Iterator: %d \n", i);
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
        //printf ("%s \n",returnvalue);
    return returnvalue;
}

extern void read_file(struct Topic **head, struct Configuration *config)
{
   
    int counter = 0;
    char *filename = "/home/studentas/Downloads/RUTX/RUTX_R_GPL_00.02.06.1/openwrt-gpl-ipq40xx-generic.Linux-x86_64/package/app_mqtt/files/mosquitto"; //"etc/config/mosquitto";   //config file location
    FILE *file;    
    file=fopen(filename,"r");               //opening file for read operation
    char *topic=NULL;
    char *qos=NULL;
    if (file == NULL){    //check if file opend
            //log_writer("Error! Could not open file"); 
            printf( "Could not open file");
            syslog(LOG_ERR, "Could not open file");
            exit(EXIT_FAILURE);
        } 
    //creating and initializing temporary string used for reading file
    char temp_string[4096];
    char second_temp_string[4096];
    memset(temp_string, 0, sizeof temp_string);
    while ((fgets(temp_string, 256, file)) != NULL) {
        if (strlen(temp_string)!=0 && strlen(temp_string)!=1) {

            if (strstr(temp_string,"config mqtt")){
                memset(temp_string, 0, sizeof temp_string); 
                fgets(temp_string, 256, file);
                if (strstr (temp_string,"option enable")){
                    config->enable = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr (temp_string,"option adress")){
                    config->address = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr (temp_string,"option local_port")){
                    config->port = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr (temp_string,"option use_tls_ssl")){
                    config->use_tls = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr (temp_string,"option tls_type")){
                    config->tls_type = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr (temp_string,"option psk")){
                    config->preshared_key= parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr (temp_string,"option identity")){
                    config->identity = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr (temp_string,"option client_enabled")){
                    config->client_enabled = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr (temp_string,"option username")){
                    config->username = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr (temp_string,"option password")){
                    config->password = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                }else if (strstr(temp_string,"config topic")){
                    memset(temp_string, 0, sizeof temp_string); 
                    fgets(temp_string, 256, file);
                    if (strstr (temp_string,"option topic")){
                        topic = parse_config_topic(temp_string);
                        fgets(temp_string, 256, file);
                        qos = parse_config_topic(temp_string); 
                        insertFirst(qos,topic, &head);
                    }else{
                        qos = parse_config_topic(temp_string);
                        fgets(temp_string, 256, file);           
                        topic = parse_config_topic(temp_string);
                        insertFirst(qos,topic, &head);
                    }
                // counter++;
                }
            }
        }
        memset(temp_string, 0, sizeof temp_string);         //clearing temporary string*/ 
    }
    fclose(file);
}

/* Main program */
int main(void)
{
    struct Topic *head = NULL;
    struct Configuration config;
    int rc =0;
    read_file(&head,&config);
    printList(&head);
    printf("%s ",config.address);
    printf("%s ",config.port);
    printf("%s ",config.username);
    printf("%s ",config.password);
    printf("%s ",config.enable);
    printf("%s ",config.use_tls);
    printf("%s ",config.tls_type);
    printf("%s ",config.tls_version);
    printf("%s ",config.preshared_key);
    printf("%s ",config.identity);
    printf("%s ",config.client_enabled);


    // char *address;
    // char *port;
    // char *username;
    // char *password;
    // char *enable;
    // char *use_tls;
    // char *tls_type;
    // char *tls_version;
    // char *preshared_key;
    // char *identity;
    // char *client_enabled;



    // struct sigaction action;
    // struct Data Connection_data;

    /* Set signal handlers */
    // signal(SIGINT, sigHandler);
    // signal(SIGTERM, sigHandler);
    // sigaction(SIGTERM, &action, NULL);


    // while(!interrupt) {   
    //    sleep(10)
    // }

//cleanUp:

    return 0;
}
