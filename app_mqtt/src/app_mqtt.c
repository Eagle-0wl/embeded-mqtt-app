#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <mosquitto.h>
#include <sqlite3.h>

#define DBPATH "/etc/app_mqtt_crt/messages.db"
#define CONFFILE "/etc/config/mosquitto_client"
#define CERTPATH "/etc/app_mqtt_crt/mosquitto.org.crt"
volatile int interrupt = 0;
sqlite3 *data_base = NULL;

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

void signal_handler(int signo) {
    signal(SIGINT, NULL);
    syslog(LOG_INFO, "Received signal: %d", signo);
    closelog();
    interrupt = 1;
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

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    struct Topic *ptr = obj;
	if(rc) {
        syslog (LOG_ERR, "Couldn't communicate to broker");
		exit(EXIT_FAILURE);
	}
    
    while(ptr != NULL) {                 //start from the beginning
        mosquitto_subscribe(mosq, NULL, ptr->topic, atoi(ptr->qos));
        ptr = ptr->next;
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    char *err = NULL;
    const char *table_query = "CREATE TABLE Messages ( \ id integer primary key autoincrement not null,\ Topic varchar(250),\ Message varchar(250),\ Time timestamp default (datetime('now', 'localtime')) not null);";
    char  *query;
    sqlite3_exec(data_base, table_query, 0, NULL, err);     //creates database table if there is none
    if (err != NULL)
        syslog(LOG_WARNING ,"Failed to insert data into database"); 
    query = sqlite3_mprintf("insert into Messages(Topic, Message) values ('%q', '%q');",(char *) msg->topic, (char *) msg->payload);
    sqlite3_exec(data_base, query, 0, NULL, err);
    sqlite3_free(query);
    if (err != NULL) 
        syslog(LOG_WARNING, "Failed to insert data into database");
}

int start_mossquitto (struct Topic *head, struct Configuration *config)
{
    int rc=0;
    struct mosquitto *mosq = NULL;

    rc = mosquitto_lib_init();
    if (rc != 0){
        syslog(LOG_ERR, "Failed to initialize mossquito");
        goto cleanup2;
    }else{
        syslog (LOG_INFO,"Mosquitto initialized successfuly");
    }
    mosq = mosquitto_new("subscribe-test", true, head);
    if (config->username!=NULL  && config->password!=NULL)  
        if (mosquitto_username_pw_set(mosq, config->username ,config->password)==MOSQ_ERR_SUCCESS)
            syslog(LOG_INFO, "User name and password added successfuly");
        else
            syslog(LOG_INFO, "Failed to add username or password");

    if (atoi(config->use_tls)==1)
        if (!mosquitto_tls_set(mosq,CERTPATH,NULL, NULL, NULL, NULL)== MOSQ_ERR_SUCCESS) 
            syslog(LOG_WARNING, "Failed to set tls");

	mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    rc = mosquitto_connect(mosq, config->address, atoi(config->port), 10);
	if(rc) {
        syslog(LOG_ERR, "Could not connect to Broker");
		goto cleanup2;
	}else{
         syslog(LOG_INFO, "Connection to broker successful");
    }
    
	rc = mosquitto_loop_start(mosq);
    if(rc) {
        syslog(LOG_ERR, "Failed to start mosquitto loop");
		goto cleanup1;
	}
    syslog (LOG_INFO,"Mosquitto loop started");

    while(!interrupt) {  }

	mosquitto_loop_stop(mosq, true);

cleanup1:
	mosquitto_disconnect(mosq);
cleanup2:
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
    return rc;
}

/* Main program */
int main(void)
{
    int rc =0;
    struct Topic *head = NULL;
    struct Topic *tmp;
    struct Configuration config;
    struct sigaction action;
    
    memset(&config, 0, sizeof(struct Configuration));
    openlog("app_mqtt", LOG_PID, LOG_USER);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    sigaction(SIGTERM, &action, NULL);

    rc = read_config_file(&head,&config);
    if (rc == -1){
        goto cleanup;
    }

    if (sqlite3_open(DBPATH, &data_base) != 0){
        syslog (LOG_ERR,"Failed to open database");
        goto cleanup;
    }
    else
        syslog (LOG_INFO,"Database opened");

    rc = start_mossquitto (head,&config);
cleanup:
    while(head != NULL) { 
        free(head->qos);
        free(head->topic);                
        tmp= head;
        head = head->next;
        free (tmp);
    }
    free (config.address);
    free (config.password);
    free (config.port);
    free (config.username);
    free (config.use_tls);
    sqlite3_close(data_base);
    return rc;   
}                                                            