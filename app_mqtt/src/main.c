#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <mqtt_db.h>
#include <mqtt_config_parsing.h>
#include <mqtt_connection.h>

/* Main program */
int main(void)
{
    int rc =0;
    struct Topic *head = NULL;
    struct Topic *tmp;
    struct Configuration config;
    
    memset(&config, 0, sizeof(struct Configuration));
    openlog("app_mqtt", LOG_PID, LOG_USER);

    rc = uci_read_config(&head,&config);
    if (rc == -1){
        goto cleanup;
    }
    
    if (open_db() == -1)
        goto cleanup;
    rc = check_if_table_exists();
    if (rc == -1){
        if (create_table() == -1)
            goto cleanup;
    }

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
    close_db();
    return rc;   
}                                                            