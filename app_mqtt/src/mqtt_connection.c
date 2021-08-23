#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <mosquitto.h>
#include <sqlite3.h>
#include <mqtt_db.h>
#include <mqtt_config_parsing.h>

#define CERTPATH "/etc/app_mqtt_crt/mosquitto.org.crt"

volatile int interrupt = 0;

void signal_handler(int signo) {
    signal(SIGINT, NULL);
    syslog(LOG_INFO, "Received signal: %d", signo);
    closelog();
    interrupt = 1;
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
    insert_message_to_database ((char *) msg->topic, (char *) msg->payload);
}

int start_mossquitto (struct Topic *head, struct Configuration *config)
{
    int rc=0;
    struct mosquitto *mosq = NULL;
    struct sigaction action;
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    sigaction(SIGTERM, &action, NULL);

    rc = mosquitto_lib_init();
    if (rc != 0){
        syslog(LOG_ERR, "Failed to initialize mossquito");
        goto cleanup2;
    }else{
        syslog (LOG_INFO,"Mosquitto initialized successfuly");
    }
    mosq = mosquitto_new("Mqtt sub program", true, head);
    if (config->username!=NULL  && config->password!=NULL) {
        if (mosquitto_username_pw_set(mosq, config->username ,config->password)==MOSQ_ERR_SUCCESS){
            syslog(LOG_INFO, "User name and password added successfuly");
        }else{
            syslog(LOG_WARNING, "Failed to add username or password");
        }
    } 
       
    if (atoi(config->use_tls)==1){
        if (!mosquitto_tls_set(mosq,CERTPATH,NULL, NULL, NULL, NULL)== MOSQ_ERR_SUCCESS){ 
            syslog(LOG_WARNING, "Failed to set tls"); 
        }
    }
    
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