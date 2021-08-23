#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <sqlite3.h>

#define DBPATH "/usr/share/messages.db"

sqlite3 *data_base = NULL;

int open_db(){
    int rc = 0;
    if (sqlite3_open(DBPATH, &data_base) != 0){
        syslog (LOG_ERR,"Failed to open database");
        rc=-1;
    }
    else
        syslog (LOG_INFO,"Database opened");
    return rc;
}

int close_db(){
    return sqlite3_close(data_base);;
}

int create_table(){
    int rc = 0;
    char *err = NULL;
    const char *table_query = "CREATE TABLE Messages ( \ id integer primary key autoincrement not null,\ Topic varchar(250),\ Message varchar(250),\ Time timestamp default (datetime('now', 'localtime')) not null);";
   
    sqlite3_exec(data_base, table_query, 0, NULL, err);     //creates database table if there is none
    if (err != NULL){
        syslog(LOG_WARNING ,"Failed to create table"); 
        sqlite3_free(err);
        rc = -1;
    }else {
        syslog(LOG_INFO ,"Table created");
    }
    return rc;
}

int check_if_table_exists(){
    int rc = 0;
    char *err = NULL;
    const char *table_query = "SELECT name FROM sqlite_master WHERE type='table' AND name='Messages';";
    struct sqlite3_stmt *selectstmt;
    rc = sqlite3_prepare_v2(data_base, table_query, -1, &selectstmt, NULL);
    if (rc == SQLITE_OK ){   
        if (sqlite3_step(selectstmt) != SQLITE_ROW)
        {
            syslog(LOG_INFO, "Table doesn't exist");
            rc = -1;
            sqlite3_free(err);
        }
    } 
    sqlite3_finalize(selectstmt);
    return rc;
}

int insert_message_to_database(char *topic, char *payload){
    char *err = NULL;
    char  *query;
    query = sqlite3_mprintf("insert into Messages(Topic, Message) values ('%q', '%q');",topic, payload);
    
    sqlite3_exec(data_base, query, 0, NULL, err);
    sqlite3_free(query);
    if (err != NULL) 
        syslog(LOG_WARNING, "Failed to insert data into database");
    return 0;
}