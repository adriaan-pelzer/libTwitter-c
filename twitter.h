#include <stdio.h>
#include <stdlib.h>
#include <oauth.h>
#include <syslog.h>
#include <Json.h>
#include <Curl.h>

#ifndef _TWITTER_H_
#define _TWITTER_H_

typedef const char *cc_p;

typedef struct twitterCtx {
    char *c_key;
    char *c_secret;
    char *t_key;
    char *t_secret;
    char *url;
    connectionType ctype;
    char *req_url;
    char *postargs;
} *twitterCtx_p;

int callTwitter(cc_p url, connectionType ctype, cc_p c_key, cc_p c_secret, cc_p t_key, cc_p t_secret, const char **paths, size_t pathcount, void *uCtx, int (*uCB)(void *uCtx, jsonStruct_p jS));
int callTwitter_progress(cc_p url, connectionType ctype, cc_p c_key, cc_p c_secret, cc_p t_key, cc_p t_secret, const char **paths, size_t pathcount, void *uCtx, int (*uCB)(void *uCtx, jsonStruct_p jS), int (*pCB)(void *uCtx, double dltotal, double dlnow, double ultotal, double ulnow));

#endif
