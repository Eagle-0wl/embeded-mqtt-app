#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <uci.h>
#include <syslog.h>
#include <mosquitto.h>

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
    //char *enable;   //nereikalingas
    char *use_tls;
    char *tls_type;
    char *tls_version;
    char *preshared_key;
    char *identity;
    //char *client_enabled; //nereikalingas

};

void printList(struct Topic *head) {
   struct Topic *ptr = head;
   
   while(ptr != NULL) {                 //start from the beginning
      printf("%s \n",ptr->topic);
      printf("%s \n",ptr->qos);
      ptr = ptr->next;
   }
}

void insertFirst(char *qos, char *combine, struct Topic **head) {
   //create a link
   size_t needed;
   struct Topic *link = (struct Topic*) malloc(sizeof(struct Topic));

   link->topic = (char*) malloc(needed);
   link->topic = combine;
   link->qos = qos;
   link->next = *head; //point it to old first node
   *head = link;       //point first to new first node
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
    //printf ("%s \n",temp_string);
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
    char *filename = "/etc/config/mosquitto";   //config file location
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
            if (strstr(temp_string,"config topic")){
                memset(temp_string, 0, sizeof temp_string); 
                fgets(temp_string, 256, file);
                if (strstr (temp_string,"option topic")){
                    topic = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);
                    qos = parse_config_topic(temp_string); 
                    insertFirst(qos,topic, head);
                }else{
                    qos = parse_config_topic(temp_string);
                    fgets(temp_string, 256, file);           
                    topic = parse_config_topic(temp_string);
                    insertFirst(qos,topic, head);
                }
            }
            /*if (strstr (temp_string,"option enable")){
                config->enable = parse_config_topic(temp_string);
            }else */if (strstr (temp_string,"option adress")){
                config->address = parse_config_topic(temp_string);
            }else if (strstr (temp_string,"option local_port")){
                config->port = parse_config_topic(temp_string);
            }else if (strstr (temp_string,"option use_tls_ssl")){
                config->use_tls = parse_config_topic(temp_string);
            }else if (strstr (temp_string,"option tls_type")){
                config->tls_type = parse_config_topic(temp_string);
            }else if (strstr (temp_string,"option psk")){
                config->preshared_key= parse_config_topic(temp_string);
            }else if (strstr (temp_string,"option identity")){
                config->identity = parse_config_topic(temp_string);
            }/*else if (strstr (temp_string,"option client_enabled")){
                config->client_enabled = parse_config_topic(temp_string);
            }*/else if (strstr (temp_string,"option username")){
                config->username = parse_config_topic(temp_string);
            }else if (strstr (temp_string,"option password")){
                config->password = parse_config_topic(temp_string);
            }
        }
        memset(temp_string, 0, sizeof temp_string);         //clearing temporary string*/ 
    }
    fclose(file);
}

extern void save_message_to_file(char *message)
{
    FILE *file;
    char *file_name="/var/mqttmessages.txt";
    if ((file = fopen(file_name, "a"))){
        fprintf(file, "%s", message);
        fclose(file);
    }else{
        printf("Error! Failed to open log file");
        exit(EXIT_FAILURE);
    }
}

void on_connect(struct mosquitto *mosq, void *obj, int rc) {

    struct Topic *ptr = obj;
	if(rc) {
		printf("Error with result code: %d\n", rc);
		exit(-1);
	}

    while(ptr != NULL) {                 //start from the beginning
        printf("%s \n",ptr->topic);
        mosquitto_subscribe(mosq, NULL, ptr->topic, 0);
        ptr = ptr->next;
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    char *message;
    size_t needed;
	printf("New message with topic %s: %s\n", msg->topic, (char *) msg->payload);

    needed = snprintf(NULL, 0, "Topic: %s Message: %s \n", msg->topic, (char *) msg->payload) + 1;
    message = (char*) malloc(needed);

    sprintf(message, "Topic: %s Message: %s \n", msg->topic, (char *) msg->payload);

    save_message_to_file (message);
    free (message);
}

/* Main program */
int main(void)
{
    struct Topic *head = NULL;
    struct Configuration config;
    struct mosquitto *mosq;
    struct sigaction action;
    int rc =0;

    memset(&config, 0, sizeof(struct Configuration));

    read_file(&head,&config);
    // printf("%s \n",config.enable); 
    // printf("%s \n",config.address);
    // printf("%s \n",config.port);
    // printf("%s \n",config.use_tls);
    // printf("%s \n",config.tls_type);
    // printf("%s \n",config.preshared_key);
    // printf("%s \n",config.identity);
    // printf("%s \n",config.client_enabled);
    // printf("%s \n",config.username);
    // printf("%s \n",config.password);
    // printf("%s \n",config.tls_version);
    
	mosquitto_lib_init();
	mosq = mosquitto_new("subscribe-test", true, head);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
    rc = mosquitto_connect(mosq, config.address, atoi(config.port), 10);
	if(rc) {
		printf("Could not connect to Broker with return code %d\n", rc);
		return -1;
	}

	mosquitto_loop_start(mosq);
	//printf("Press Enter to quit...\n");
	//getchar();

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    sigaction(SIGTERM, &action, NULL);
    while(!interrupt) {  
       //sleep(10)
    }
	mosquitto_loop_stop(mosq, true);


//cleanUp:
	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
    return 0;
}
