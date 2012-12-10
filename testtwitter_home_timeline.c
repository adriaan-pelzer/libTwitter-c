#include <Twitter.h>
#include "addr.h"

#define DAEMON_NAME "libTwitter"

static int count = 0;

int tweetCB_array(void *uCtx, jsonStruct_p jS) {
    int rc = -1;
    size_t *count = (size_t *) uCtx;
    size_t _count = *count;
    jsonElm_p element = NULL;
    char path[256];
    size_t i = 0;

    snprintf(path, 256, "root");
    DOA("get root (tweet count)", getElement, element, NULL, jS, path);
    if (element->type != json_type_array) {
        syslog(P_ERR, "root has the wrong type");
        goto over;
    }

    for (i = 0; i < (size_t) element->value.intVal; i++) {
        snprintf(path, 256, "root|%d|text", (int) i);
        DOA("get root (tweet count)", getElement, element, NULL, jS, path);
        if (element->type != json_type_string) {
            syslog(P_ERR, "%s has the wrong type", path);
            goto over;
        }
        if (element->value.stringVal) { printf("%d: %s\n\n", (int) _count, element->value.stringVal); _count ++; }
    }

    *count = _count;
    rc = 0;
over:
    return rc;
}

int main(void) {
    int rc = EXIT_FAILURE;
    const char *c_key = "EJT0zK8FVwPWxOplnqs6tQ";
    const char *c_secret = "UArI5SEP3kjJJ3Z4li7DndPXiYu0xs6bDcAew9o8OTA";
    const char *t_key = "169026281-ZIpG6KuViclMLOf8zDIFSNjWOUznFG5MrVupszOD";
    const char *t_secret = "tcTcuZN6MWDC8UBehnpY66pDCT7GSN0dxqo8Pddcg";
    const char *url = "https://api.twitter.com/1.1/statuses/home_timeline.json";
    connectionType ctype = CONN_GET;
    const char *path[] = {
        "root",
        "root|%d",
        "root|%d|text"
    };
    size_t count = 0;

#ifdef DEBUG
    setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog (DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
#else
    setlogmask (LOG_UPTO (LOG_INFO));
    openlog (DAEMON_NAME, LOG_CONS, LOG_USER);
#endif

    DONT("call twitter", callTwitter, 0, url, ctype, c_key, c_secret, t_key, t_secret, path, 3, (void *) &count, tweetCB_array);

    rc = EXIT_SUCCESS;
over:
    return rc;
}
