#include <sys/wait.h>
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <engine_loader.h>
#include <memcached/engine_testapp.h>
#include <memcached/extension_loggers.h>
#include <memcached/mock_server.h>

struct mock_engine {
    ENGINE_HANDLE_V1 me;
    ENGINE_HANDLE_V1 *the_engine;
};

static inline struct mock_engine* get_handle(ENGINE_HANDLE* handle) {
    return (struct mock_engine*)handle;
}

static const engine_info* mock_get_info(ENGINE_HANDLE* handle) {
    struct mock_engine *me = get_handle(handle);
    return me->the_engine->get_info((ENGINE_HANDLE*)me->the_engine);
}

static ENGINE_ERROR_CODE mock_initialize(ENGINE_HANDLE* handle,
                                         const char* config_str) {
    struct mock_engine *me = get_handle(handle);
    return me->the_engine->initialize((ENGINE_HANDLE*)me->the_engine, config_str);
}

static void mock_destroy(ENGINE_HANDLE* handle) {
    struct mock_engine *me = get_handle(handle);
    me->the_engine->destroy((ENGINE_HANDLE*)me->the_engine);
}

static ENGINE_ERROR_CODE mock_allocate(ENGINE_HANDLE* handle,
                                       const void* cookie,
                                       item **item,
                                       const void* key,
                                       const size_t nkey,
                                       const size_t nbytes,
                                       const int flags,
                                       const rel_time_t exptime) {
    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->allocate((ENGINE_HANDLE*)me->the_engine, c,
                                           item, key, nkey,
                                           nbytes, flags,
                                           exptime)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}

static ENGINE_ERROR_CODE mock_remove(ENGINE_HANDLE* handle,
                                     const void* cookie,
                                     const void* key,
                                     const size_t nkey,
                                     uint64_t cas,
                                     uint16_t vbucket)
{
    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->remove((ENGINE_HANDLE*)me->the_engine, c, key,
                                         nkey, cas, vbucket)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}

static void mock_release(ENGINE_HANDLE* handle,
                         const void *cookie,
                         item* item) {
    struct mock_engine *me = get_handle(handle);
    me->the_engine->release((ENGINE_HANDLE*)me->the_engine, cookie, item);
}

static ENGINE_ERROR_CODE mock_get(ENGINE_HANDLE* handle,
                                  const void* cookie,
                                  item** item,
                                  const void* key,
                                  const int nkey,
                                  uint16_t vbucket) {
    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->get((ENGINE_HANDLE*)me->the_engine, c, item,
                                      key, nkey, vbucket)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}

static ENGINE_ERROR_CODE mock_get_stats(ENGINE_HANDLE* handle,
                                        const void* cookie,
                                        const char* stat_key,
                                        int nkey,
                                        ADD_STAT add_stat)
{
    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->get_stats((ENGINE_HANDLE*)me->the_engine, c, stat_key,
                                            nkey, add_stat)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}

static ENGINE_ERROR_CODE mock_store(ENGINE_HANDLE* handle,
                                    const void *cookie,
                                    item* item,
                                    uint64_t *cas,
                                    ENGINE_STORE_OPERATION operation,
                                    uint16_t vbucket) {
    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->store((ENGINE_HANDLE*)me->the_engine, c, item, cas,
                                        operation, vbucket)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}

static ENGINE_ERROR_CODE mock_arithmetic(ENGINE_HANDLE* handle,
                                         const void* cookie,
                                         const void* key,
                                         const int nkey,
                                         const bool increment,
                                         const bool create,
                                         const uint64_t delta,
                                         const uint64_t initial,
                                         const rel_time_t exptime,
                                         uint64_t *cas,
                                         uint64_t *result,
                                         uint16_t vbucket) {
    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->arithmetic((ENGINE_HANDLE*)me->the_engine, c, key,
                                             nkey, increment, create,
                                             delta, initial, exptime,
                                             cas, result, vbucket)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}

static ENGINE_ERROR_CODE mock_flush(ENGINE_HANDLE* handle,
                                    const void* cookie, time_t when) {
    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->flush((ENGINE_HANDLE*)me->the_engine, c, when)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}

static void mock_reset_stats(ENGINE_HANDLE* handle, const void *cookie) {
    struct mock_engine *me = get_handle(handle);
    me->the_engine->reset_stats((ENGINE_HANDLE*)me->the_engine, cookie);
}

static ENGINE_ERROR_CODE mock_unknown_command(ENGINE_HANDLE* handle,
                                              const void* cookie,
                                              protocol_binary_request_header *request,
                                              ADD_RESPONSE response)
{
    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->unknown_command((ENGINE_HANDLE*)me->the_engine, c,
                                                  request, response)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}

static void mock_item_set_cas(ENGINE_HANDLE *handle, item* item, uint64_t val)
{
    struct mock_engine *me = get_handle(handle);
    me->the_engine->item_set_cas((ENGINE_HANDLE*)me->the_engine, item, val);
}


static bool mock_get_item_info(ENGINE_HANDLE *handle, const item* item, item_info *item_info)
{
    struct mock_engine *me = get_handle(handle);
    return me->the_engine->get_item_info((ENGINE_HANDLE*)me->the_engine, item, item_info);
}

static void *mock_get_stats_struct(ENGINE_HANDLE* handle, const void* cookie)
{
    struct mock_engine *me = get_handle(handle);
    return me->the_engine->get_stats_struct((ENGINE_HANDLE*)me->the_engine, cookie);
}

static ENGINE_ERROR_CODE mock_aggregate_stats(ENGINE_HANDLE* handle,
                                              const void* cookie,
                                              void (*callback)(void*, void*),
                                              void *vptr)
{
    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->aggregate_stats((ENGINE_HANDLE*)me->the_engine, c,
                                                  callback, vptr)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}

static ENGINE_ERROR_CODE mock_tap_notify(ENGINE_HANDLE* handle,
                                        const void *cookie,
                                        void *engine_specific,
                                        uint16_t nengine,
                                        uint8_t ttl,
                                        uint16_t tap_flags,
                                        tap_event_t tap_event,
                                        uint32_t tap_seqno,
                                        const void *key,
                                        size_t nkey,
                                        uint32_t flags,
                                        uint32_t exptime,
                                        uint64_t cas,
                                        const void *data,
                                        size_t ndata,
                                         uint16_t vbucket) {

    struct mock_engine *me = get_handle(handle);
    struct mock_connstruct *c = (void*)cookie;
    if (c == NULL) {
        c = (void*)create_mock_cookie();
    }

    c->nblocks = 0;
    ENGINE_ERROR_CODE ret = ENGINE_SUCCESS;
    pthread_mutex_lock(&c->mutex);
    while (ret == ENGINE_SUCCESS &&
           (ret = me->the_engine->tap_notify((ENGINE_HANDLE*)me->the_engine, c,
                                             engine_specific, nengine, ttl, tap_flags,
                                             tap_event, tap_seqno, key, nkey, flags,
                                             exptime, cas, data, ndata, vbucket)) == ENGINE_EWOULDBLOCK &&
           c->handle_ewouldblock)
    {
        ++c->nblocks;
        pthread_cond_wait(&c->cond, &c->mutex);
        ret = c->status;
    }
    pthread_mutex_unlock(&c->mutex);

    if (c != cookie) {
        destroy_mock_cookie(c);
    }

    return ret;
}


static TAP_ITERATOR mock_get_tap_iterator(ENGINE_HANDLE* handle, const void* cookie,
                                           const void* client, size_t nclient,
                                           uint32_t flags,
                                           const void* userdata, size_t nuserdata) {
    struct mock_engine *me = get_handle(handle);
    return me->the_engine->get_tap_iterator((ENGINE_HANDLE*)me->the_engine, cookie,
                                            client, nclient, flags, userdata, nuserdata);
}

static size_t mock_errinfo(ENGINE_HANDLE *handle, const void* cookie,
                           char *buffer, size_t buffsz) {
    struct mock_engine *me = get_handle(handle);
    return me->the_engine->errinfo((ENGINE_HANDLE*)me->the_engine, cookie,
                                   buffer, buffsz);
}


struct mock_engine mock_engine = {
    .me = {
        .interface = {
            .interface = 1
        },
        .get_info = mock_get_info,
        .initialize = mock_initialize,
        .destroy = mock_destroy,
        .allocate = mock_allocate,
        .remove = mock_remove,
        .release = mock_release,
        .get = mock_get,
        .store = mock_store,
        .arithmetic = mock_arithmetic,
        .flush = mock_flush,
        .get_stats = mock_get_stats,
        .reset_stats = mock_reset_stats,
        .get_stats_struct = mock_get_stats_struct,
        .aggregate_stats = mock_aggregate_stats,
        .unknown_command = mock_unknown_command,
        .tap_notify = mock_tap_notify,
        .get_tap_iterator = mock_get_tap_iterator,
        .item_set_cas = mock_item_set_cas,
        .get_item_info = mock_get_item_info,
        .errinfo = mock_errinfo
    }
};


EXTENSION_LOGGER_DESCRIPTOR *logger_descriptor = NULL;
static ENGINE_HANDLE *handle = NULL;
static ENGINE_HANDLE_V1 *handle_v1 = NULL;

static void usage(void) {
    printf("\n");
    printf("engine_testapp -E <path_to_engine_lib> -T <path_to_testlib>\n");
    printf("               [-e <engine_config>] [-h]\n");
    printf("\n");
    printf("-E <path_to_engine_lib>      Path to the engine library file. The\n");
    printf("                             engine library file is a library file\n");
    printf("                             (.so or .dll) that the contains the \n");
    printf("                             implementation of the engine being\n");
    printf("                             tested.\n");
    printf("\n");
    printf("-T <path_to_testlib>         Path to the test library file. The test\n");
    printf("                             library file is a library file (.so or\n");
    printf("                             .dll) that contains the set of tests\n");
    printf("                             to be executed.\n");
    printf("\n");
    printf("-e <engine_config>           Engine configuration string passed to\n");
    printf("                             the engine.\n");
    printf("\n");
    printf("-h                           Prints this usage text.\n");
    printf("\n");
}

static int report_test(enum test_result r) {
    int rc = 0;
    char *msg = NULL;
    bool color_enabled = getenv("TESTAPP_ENABLE_COLOR") != NULL;
    int color = 0;
    char color_str[8] = { 0 };
    char *reset_color = "\033[m";
    switch(r) {
    case SUCCESS:
        msg="OK";
        color = 32;
        break;
    case FAIL:
        color = 31;
        msg="FAIL";
        rc = 1;
        break;
    case DIED:
        color = 31;
        msg = "DIED";
        rc = 1;
        break;
    case CORE:
        color = 31;
        msg = "CORE DUMPED";
        rc = 1;
        break;
    case PENDING:
        color = 33;
        msg = "PENDING";
        break;
    }
    assert(msg);
    if (color_enabled) {
        snprintf(color_str, sizeof(color_str), "\033[%dm", color);
    }
    printf("%s%s%s\n", color_str, msg, color_enabled ? reset_color : "");
    return rc;
}

static ENGINE_HANDLE_V1 *start_your_engines(const char *engine, const char* cfg, bool engine_init) {

    init_mock_server(handle);
    if (!load_engine(engine, &get_mock_server_api, logger_descriptor, &handle)) {
        fprintf(stderr, "Failed to load engine %s.\n", engine);
        return NULL;
    }

    if (engine_init) {
        if(!init_engine(handle, cfg, logger_descriptor)) {
            fprintf(stderr, "Failed to init engine %s with config %s.\n", engine, cfg);
            return NULL;
        }
    }

    handle_v1 = mock_engine.the_engine = (ENGINE_HANDLE_V1*)handle;
    handle = (ENGINE_HANDLE*)&mock_engine.me;
    handle_v1 = &mock_engine.me;
    return &mock_engine.me;
}

static void destroy_engine() {
    if (handle_v1) {
        handle_v1->destroy(handle);
        handle_v1 = NULL;
        handle = NULL;
    }
}

static void reload_engine(ENGINE_HANDLE **h, ENGINE_HANDLE_V1 **h1, const char* engine, const char *cfg, bool init) {
    destroy_engine();
    handle_v1 = start_your_engines(engine, cfg, init);
    handle = (ENGINE_HANDLE*)(handle_v1);
    *h1 = handle_v1;
    *h = handle;
}

static enum test_result run_test(engine_test_t test, const char *engine, const char *default_cfg) {
    enum test_result ret = PENDING;
    if (test.tfun != NULL) {
#ifndef USE_GCOV
        pid_t pid = fork();
        if (pid == 0) {
#endif
            /* Start the engines and go */
            start_your_engines(engine, test.cfg ? test.cfg : default_cfg, true);
            if (test.test_setup != NULL) {
                if (!test.test_setup(handle, handle_v1)) {
                    fprintf(stderr, "Failed to run setup for test %s\n", test.name);
                    return FAIL;
                }
            }
            ret = test.tfun(handle, handle_v1);
            if (test.test_teardown != NULL) {
                if (!test.test_teardown(handle, handle_v1)) {
                    fprintf(stderr, "WARNING: Failed to run teardown for test %s\n", test.name);
                }
            }
            destroy_engine();
#ifndef USE_GCOV
            exit((int)ret);
        } else if (pid == (pid_t)-1) {
            ret = FAIL;
        } else {
            int rc;
            while (waitpid(pid, &rc, 0) == (pid_t)-1) {
                if (errno != EINTR) {
                    abort();
                }
            }

            if (WIFEXITED(rc)) {
                ret = (enum test_result)WEXITSTATUS(rc);
            } else if (WIFSIGNALED(rc) && WCOREDUMP(rc)) {
                ret = CORE;
            } else {
                ret = DIED;
            }
        }
#endif
    }

    return ret;
}


int main(int argc, char **argv) {
    int c, exitcode = 0, num_cases = 0;
    const char *engine = NULL;
    const char *engine_args = NULL;
    const char *test_suite = NULL;
    engine_test_t *testcases = NULL;
    logger_descriptor = get_null_logger();

    /* Hack to remove the warning from C99 */
    union {
        GET_TESTS get_tests;
        void* voidptr;
    } my_get_test = {.get_tests = NULL };

    /* Hack to remove the warning from C99 */
    union {
        SETUP_SUITE setup_suite;
        void* voidptr;
    } my_setup_suite = {.setup_suite = NULL };

    /* Hack to remove the warning from C99 */
    union {
        TEARDOWN_SUITE teardown_suite;
        void* voidptr;
    } my_teardown_suite = {.teardown_suite = NULL };


    /* Use unbuffered stdio */
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    /* process arguments */
    while (-1 != (c = getopt(argc, argv,
          "h"  /* usage */
          "E:"  /* Engine to load */
          "e:"  /* Engine options */
          "T:"   /* Library with tests to load */
        ))) {
        switch (c) {
        case 'E':
            engine = optarg;
            break;
        case 'e':
            engine_args = optarg;
            break;
        case 'h':
            usage();
            return 0;
        case 'T':
            test_suite = optarg;
            break;
        default:
            fprintf(stderr, "Illegal argument \"%c\"\n", c);
            return 1;
        }
    }

    //validate args
    if (engine == NULL) {
        fprintf(stderr, "You must provide a path to the storage engine library.\n");
        return 1;
    }

    if (test_suite == NULL) {
        fprintf(stderr, "You must provide a path to the testsuite library.\n");
        return 1;
    }

    //load test_suite
    void* handle = dlopen(test_suite, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
        const char *msg = dlerror();
        fprintf(stderr, "Failed to load testsuite %s: %s\n", test_suite, msg ? msg : "unknown error");
        return 1;
    }

    //get the test cases
    void *symbol = dlsym(handle, "get_tests");
    if (symbol == NULL) {
        const char *msg = dlerror();
        fprintf(stderr, "Could not find get_tests function in testsuite %s: %s\n", test_suite, msg ? msg : "unknown error");
        return 1;
    }
    my_get_test.voidptr = symbol;
    testcases = (*my_get_test.get_tests)();

    //set up the suite if needed
    struct test_harness harness = { .default_engine_cfg = engine_args,
                                    .engine_path = engine,
                                    .reload_engine = reload_engine,
                                    .start_engine = start_your_engines,
                                    .create_cookie = create_mock_cookie,
                                    .destroy_cookie = destroy_mock_cookie,
                                    .set_ewouldblock_handling = mock_set_ewouldblock_handling};
    symbol = dlsym(handle, "setup_suite");
    if (symbol != NULL) {
        my_setup_suite.voidptr = symbol;
        if (!(*my_setup_suite.setup_suite)(&harness)) {
            fprintf(stderr, "Failed to set up test suite %s \n", test_suite);
            return 1;
        }
    }


    for (num_cases = 0; testcases[num_cases].name; num_cases++) {
        /* Just counting */
    }

    printf("1..%d\n", num_cases);

    int i;
    for (i = 0; testcases[i].name; i++) {
        printf("Running %s... ", testcases[i].name);
        fflush(stdout);
        exitcode += report_test(run_test(testcases[i], engine, engine_args));
    }

    //tear down the suite if needed
    symbol = dlsym(handle, "teardown_suite");
    if (symbol != NULL) {
        my_teardown_suite.voidptr = symbol;
        if (!(*my_teardown_suite.teardown_suite)()) {
            fprintf(stderr, "Failed to teardown up test suite %s \n", test_suite);
        }
    }

    return exitcode;
}