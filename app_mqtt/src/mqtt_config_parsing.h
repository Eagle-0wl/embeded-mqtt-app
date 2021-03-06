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

void insert_topic_into_linked_list(char *qos, char *topic, struct Topic **head);
int uci_read_config(struct Topic **head, struct Configuration *config);

// void insert_data_to_linked_list(char *qos, char *combine, struct Topic **head);
// char *parse_data_from_config(char *temp_string);
// int read_config_file(struct Topic **head, struct Configuration *config);