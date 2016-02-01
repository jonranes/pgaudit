/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* libcurl (http://curl.haxx.se/libcurl/c) */
#include <curl/curl.h>

/* holder for curl fetch */
struct curl_fetch_st {
    char *payload;
    size_t size;
};

/* callback for curl fetch */
size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;                             /* calculate buffer size */
    struct curl_fetch_st *p = (struct curl_fetch_st *) userp;   /* cast pointer to fetch struct */

    /* expand buffer */
    p->payload = (char *) realloc(p->payload, p->size + realsize + 1);

    /* check buffer */
    if (p->payload == NULL) {
      /* this isn't good */
      fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
      /* free buffer */
      free(p->payload);
      /* return */
      return -1;
    }

    /* copy contents to buffer */
    memcpy(&(p->payload[p->size]), contents, realsize);

    /* set new buffer size */
    p->size += realsize;

    /* ensure null termination */
    p->payload[p->size] = 0;

    /* return size */
    return realsize;
}

/* fetch and return url body via curl */
CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch) {
    CURLcode rcode;                   /* curl result code */

    /* init payload */
    fetch->payload = (char *) calloc(1, sizeof(fetch->payload));

    /* check payload */
    if (fetch->payload == NULL) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to allocate payload in curl_fetch_url");
        /* return error */
        return CURLE_FAILED_INIT;
    }

    /* init size */
    fetch->size = 0;

    /* set url to fetch */
    curl_easy_setopt(ch, CURLOPT_URL, url);

    /* set calback function */
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);

    /* pass fetch struct pointer */
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) fetch);

    /* set default user agent */
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* set timeout */
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, 5);

    /* enable location redirects */
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);

    /* set maximum allowed redirects */
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);

    /* fetch the url */
    rcode = curl_easy_perform(ch);

    /* return */
    return rcode;
}

char* concat(char *s1, char *s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *result = malloc(len1+len2+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+1);//+1 to copy the null-terminator
    return result;
}

int* post_gelf(char* timestamp_val, char* database_val, char* username, char* eusername, char* classname, char* tag, char* object_type, char* object_id, char* command_text) {

    //char* fullstring = "{\"short_message\":\"pg\", \"host\":\"wpo-vmdb01\", \"facility\":\"GELF\", \"version\":\"1.0\"}";
    
    char* first= "{\"short_message\":\"pg\", \"host\":\"yourservername\", \"facility\":\"GELF\", \"version\":\"1.0\"";

    char* tmp = concat(",\"DatabaseVal\": \"", database_val);
    char* database = concat(tmp, "\"");
    free(tmp);
    tmp = concat(first, database);
    free(database);    

    char* tmp2 = concat(",\"UserName\": \"", username);
    char* username_val = concat(tmp2, "\"");
    free(tmp2);
    tmp2 = concat(tmp, username_val);
    free(tmp);
    free(username_val);

    char* tmp3 = concat(",\"Eusername\": \"", eusername);
    char* eusername_val = concat(tmp3, "\"");
    free(tmp3);
    tmp3 = concat(tmp2, eusername_val);
    free(tmp2);
    free(eusername_val);

    char* tmp4 = concat(",\"Classname\": \"", classname);
    char* classname_val = concat(tmp4, "\"");
    free(tmp4);
    tmp4 = concat(tmp3, classname_val);
    free(tmp3);
    free(classname_val);

    char* tmp5 = concat(",\"Tag\": \"", tag);
    char* tag_val = concat(tmp5, "\"");
    free(tmp5);
    tmp5 = concat(tmp4, tag_val);
    free(tmp4);
    free(tag_val);

    char* tmp6 = concat(",\"ObjectType\": \"", object_type);
    char* object_type_val = concat(tmp6, "\"");
    free(tmp6);
    tmp6 = concat(tmp5, object_type_val);
    free(tmp5);
    free(object_type_val);

    char* tmp7 = concat(",\"ObjectId\": \"", object_id);
    char* object_id_val = concat(tmp7, "\"");
    free(tmp7);
    tmp7 = concat(tmp6, object_id_val);
    free(tmp6);
    free(object_id_val);

    char* tmp8 = concat(",\"CommandText\": \"", command_text);
    char* command_text_val = concat(tmp8, "\"");
    free(tmp8);
    tmp8 = concat(tmp7, command_text_val);
    free(tmp7);
    free(command_text_val);

    char* last = "}";
    char* json_to_send = concat(tmp8, last);
    free(tmp8);

    //fprintf(stderr, json_to_send);

    CURL *ch;                                               /* curl handle */
    CURLcode rcode;                                         /* curl result code */

    struct curl_fetch_st curl_fetch;                        /* curl fetch struct */
    struct curl_fetch_st *cf = &curl_fetch;                 /* pointer to fetch struct */
    struct curl_slist *headers = NULL;                      /* http headers to send with request */

    /* url to test site */
    char *url = "http://127.0.0.1:12201/gelf";

    /* init curl handle */
    if ((ch = curl_easy_init()) == NULL) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to create curl handle in fetch_session");
        /* return error */
        return 1;
    }

    /* set content type */
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    /* set curl options */
    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, json_to_send);

    /* fetch page and capture return code */
    rcode = curl_fetch_url(ch, url, cf);

    /* cleanup curl handle */
    curl_easy_cleanup(ch);

    /* free headers */
    curl_slist_free_all(headers);

    /* check return code */
    if (rcode != CURLE_OK) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s",
            url, curl_easy_strerror(rcode));
        /* return error */
        return 2;
    }

    free(json_to_send);

    /* exit */
    return 0;
}

