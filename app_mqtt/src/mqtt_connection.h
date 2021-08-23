void signal_handler(int signo);
void on_connect(struct mosquitto *mosq, void *obj, int rc);
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
int start_mossquitto (struct Topic *head, struct Configuration *config);