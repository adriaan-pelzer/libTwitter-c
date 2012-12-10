#include <Twitter.h>
#include "addr.h"

#define DAEMON_NAME "libTwitter"

static int count = 0;

int tweetCB_single(void *uCtx, jsonStruct_p jS) {
    int rc = -1;
    size_t *count = (size_t *) uCtx;
    size_t _count = *count;
    jsonElm_p element = NULL;
    char path[256];

    snprintf(path, 256, "root|text");
    DOA("get tweet text", getElement, element, NULL, jS, path);
    if (element->type != json_type_string) {
        syslog(P_ERR, "tweet text has the wrong type");
        goto over;
    }

    if (element->value.stringVal) { printf("%d: %s\n\n", (int) _count, element->value.stringVal); _count ++; }

    *count = _count;
    if (_count > 50) goto over;
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
    const char *url = "https://stream.twitter.com/1.1/statuses/sample.json";
    connectionType ctype = CONN_GET;
    const char *path[] = { 
        "root",
        "root|text" 
    };
    size_t count = 0;

#ifdef DEBUG
    setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog (DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
#else
    setlogmask (LOG_UPTO (LOG_INFO));
    openlog (DAEMON_NAME, LOG_CONS, LOG_USER);
#endif

    DONT("call twitter", callTwitter, 0, url, ctype, c_key, c_secret, t_key, t_secret, path, 2, (void *) &count, tweetCB_single);

    rc = EXIT_SUCCESS;
over:
    return rc;
}
