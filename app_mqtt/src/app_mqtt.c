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
    int qos;
    struct Topic* next;
}

struct Configuration {
    char *address;
    char *port;
    char *username;
    char *password;
    int enable;
    int use_tsl;
    char *tls_type;
    char *tls_version;
    char *preshared_key;
    char *identity;
};

void printList(struct Node* n)
{
    while (n != NULL) {
        printf(" %d ", n->data);
        n = n->next;
    }
}
 

void sigHandler(int signo) {
    signal(SIGINT, NULL);
    syslog(LOG_INFO, "Received signal: %d", signo);
    closelog();
    interrupt = 1;
}

char *remove_white_spaces(char *str)
{
	int i = 0, j = 0;
	while (str[i]){
		if (str[i] != ' '  && str[i] != '\n')
          str[j++] = str[i];
		i++;
	}
	str[j] = '\0';
	return str;
}


void parse_config_topic(char *temp_string,*second_temp_string,struct Topic ){
    int quotes_counter=0;
    if (strstr(temp_string,"option topic")){
        for (int i=0;i <strlen(temp_string); i++){
            if (temp_string[i] == '\'')
                quotes_counter++;
            if (temp_string[i] != '\'' && quotes_counter == 1 ){

            }else if (temp_string[i] == '\'' && quotes_counter == 2){
                quotes_counter=0;
            }
        }

    }else if (strstr(second_temp_string, "option qos")){

    }
    if (strstr(temp_string,"option qos")){

    }else if (strstr(second_temp_string, "option qos")){

    }

}

extern void read_file(struct Topic  ,struct Configuration )
{
    struct Topic topics[100];
    int counter = 0;
    char *filename = "/home/studentas/Downloads/RUTX/RUTX_R_GPL_00.02.06.1/openwrt-gpl-ipq40xx-generic.Linux-x86_64/package/app_mqtt/files/mosquitto"; //"etc/config/mosquitto";   //config file location
    FILE *file;    
    file=fopen(filename,"r");               //opening file for read operation
    if (file == NULL){    //check if file opend
            //log_writer("Error! Could not open file"); 
            syslog(LOG_ERR, "Could not open file");
            exit(EXIT_FAILURE);
        } 
    //creating and initializing temporary string used for reading file
    char temp_string[4096];
    char second_temp_string[4096];
    memset(temp_string, 0, sizeof temp_string);
    memset(temp_string, 0, sizeof second_temp_string);

    while ((fgets(temp_string, 256, file)) != NULL) {
        //remove_white_spaces (temp_string);

        if (strlen(temp_string)!=0 && !strstr (temp_string,"\n")) {
            if (strstr(temp_string,"config topic")){
                fgets(temp_string, 256, file);
                fgets(second_temp_string, 256, file);
                parse_config_topic(temp_string,second_temp_string);
                counter++;
            }

        }
        memset(temp_string, 0, sizeof temp_string);         //clearing temporary string*/ 
        memset(temp_string, 0, sizeof second_temp_string);
    }

    // if (strlen(type_to_watch[0]) == 0 && strlen(directory[0]) == 0){     //checks if config file read
    //     log_writer ("Somethings wrong with config file!");  
    //     fclose(file);                           //close file
    //     exit(EXIT_FAILURE);
    // }
    // else {
    //     log_writer("Config file read correctly!");
    // }
    free (temp_string);
    free (second_temp_string);
    fclose(file);
}









/* Main program */
int main(void)
{
    int rc =0;
    uint32_t id;
    struct Configuration *config = malloc(sizeof *config);


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
