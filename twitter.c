#include "addr.h"
#include "twitter.h"

static int (*userCB)(void *uCtx, jsonStruct_p jS);
static char **whitelisted_paths;
static size_t whitelist_size;

static int replace_str(char *str, char *orig, char *rep) {
    int rc = -1;
    char *p;
    size_t i = 0, j = 0, startofevil = 0, endofevil = 0, length = 0;

    if((p = strstr(str, orig))) {
        startofevil = (p - str) + strlen(rep);
        endofevil = (p - str) + strlen(orig);
        length = strlen(str);

        snprintf(p, strlen(rep) + 1, "%s", rep);
        for (i = startofevil, j = endofevil; j <= length; i++, j++) {
            str[i] = str[j];
        }
        goto over;
    }

    rc = 0;
over:
    return rc;
}

static void clean_json(char *memory, size_t *size) {
    while (strstr(memory, "\\u003C")) {
        if (replace_str(memory, "\\u003C", "<") == 0) {
            break;
        } else {
            *size = strlen(memory);
        }
    }

    while (strstr(memory, "\\u003E")) {
        if (replace_str(memory, "\\u003E", ">") == 0) {
            break;
        } else {
            *size = strlen(memory);
        }
    }

    while (strstr(memory, "\\u007B")) {
        if (replace_str(memory, "\\u007B", "{") == 0) {
            break;
        } else {
            *size = strlen(memory);
        }
    }

    while (strstr(memory, "\\u007D")) {
        if (replace_str(memory, "\\u007D", "}") == 0) {
            break;
        } else {
            *size = strlen(memory);
        }
    }
}

void freeTwitterCtx(twitterCtx_p tCtx) {
    if (tCtx) {
        F(tCtx->c_key);
        F(tCtx->c_secret);
        F(tCtx->t_key);
        F(tCtx->t_secret);
        F(tCtx->url);
        F(tCtx->req_url);
        F(tCtx->postargs);
        F(tCtx);
    }
    return;
}

twitterCtx_p createTwitterCtx(const char *c_key, const char *c_secret, const char *t_key, const char *t_secret) {
    twitterCtx_p rc = NULL, _rc = NULL;

    DOA("allocate memory for twitter context", malloc, _rc, NULL, sizeof(struct twitterCtx));
    memset(_rc, 0, sizeof(struct twitterCtx));
    DOA("allocate memory for consumer key", strdup, _rc->c_key, NULL, c_key);
    DOA("allocate memory for consumer secret", strdup, _rc->c_secret, NULL, c_secret);
    DOA("allocate memory for user key", strdup, _rc->t_key, NULL, t_key);
    DOA("allocate memory for user secret", strdup, _rc->t_secret, NULL, t_secret);

    rc = _rc;
over:
    FFF(_rc, freeTwitterCtx, _rc && (rc == NULL));
    return rc;
}

static int setUrl(twitterCtx_p tCtx, const char *url, connectionType ctype) {
    int rc = -1;

    F(tCtx->url);
    DOA("allocate memory for url", strdup, tCtx->url, NULL, url);
    tCtx->ctype = ctype;

    rc = 0;
over:
    return rc;
}

static int streamCB(void *uCtx, char **memory, size_t *size) {
    int rc = -1;
    char *p = NULL;
    char *first = NULL;
    char *last = NULL;
    jsonStruct_p jS = NULL;

    if (strlen(*memory) != *size) {
        syslog(P_ERR, "Size mismatch");
        goto over;
    }

    if (*size > 2) {
        while((p = strchr(*memory, '\n'))) {
            size_t len = ((size_t) p - (size_t) *memory);

            DOAS("allocate memory for single tweet JSON", malloc, first, NULL, len + 1);
            DOAS("allocate memory for remaining JSON", malloc, last, NULL, *size - len);
            snprintf(first, len + 1, "%s", *memory);
            snprintf(last, *size - len + 1, "%s", p + 1);

            clean_json(first, &len);

            /* convert */
            syslog(P_DBG, "Content: %s", first);
            DOASD("get tweet json struct", parseJson, jS, NULL, first, (const char **) whitelisted_paths, whitelist_size);

            if (jS) { DONT("call user callback function on json struct", userCB, 0, uCtx, jS); }

            F(first); first = NULL;
            F(*memory); *memory = last; last = NULL;
            *size = strlen(*memory);

            freeJsonStruct(jS); jS = NULL;
        }
    }

    rc = 0;
over:
    F(first);
    F(last);
    freeJsonStruct(jS);
    return rc;
}

static int returnCB(void *uCtx, const char *memory, const size_t size) {
    int rc = -1;
    char *_memory;
    size_t _size = size;
    jsonStruct_p jS = NULL;

    if (strlen(memory) != size) {
        syslog(P_ERR, "Size mismatch");
        goto over;
    }

    if (size > 2) {
        DOAS("allocate memory for memory", strdup, _memory, NULL, memory);

        clean_json(_memory, &_size);

        syslog(P_DBG, "Content: %s", _memory);
        DOA("get tweet json struct", parseJson, jS, NULL, _memory, (const char **) whitelisted_paths, whitelist_size);

        if (jS) { DONT("call user callback function on json struct", userCB, 0, uCtx, jS); }
    }

    rc = 0;
over:
    F(_memory);
    freeJsonStruct(jS);
    return rc;
}

int callTwitter(cc_p url, connectionType ctype, cc_p c_key, cc_p c_secret, cc_p t_key, cc_p t_secret, const char **paths, size_t pathcount, void *uCtx, int (*uCB)(void *uCtx, jsonStruct_p jS)) {
    int rc = -1;
    twitterCtx_p tCtx = NULL;

    userCB = uCB;
    whitelisted_paths = (char **) paths;
    whitelist_size = pathcount;
    DOA("allocate memory for twitter context", createTwitterCtx, tCtx, NULL, c_key, c_secret, t_key, t_secret);
    DONT("set twitter url", setUrl, 0, tCtx, url, ctype);

    if (tCtx->ctype == CONN_POST)
        tCtx->req_url = oauth_sign_url2(tCtx->url, &tCtx->postargs, OA_HMAC, NULL, tCtx->c_key, tCtx->c_secret, tCtx->t_key, tCtx->t_secret);
    else
        tCtx->req_url = oauth_sign_url2(tCtx->url, NULL, OA_HMAC, NULL, tCtx->c_key, tCtx->c_secret, tCtx->t_key, tCtx->t_secret);

    DONT("connect", curl_connect, 0, tCtx->req_url, tCtx->ctype, tCtx->postargs, uCtx, returnCB, streamCB);

    rc = 0;
over:
    FF(tCtx, freeTwitterCtx);
    return rc;
}
