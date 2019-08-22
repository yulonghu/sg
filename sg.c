/*
  +----------------------------------------------------------------------+
  | SG - A Simple PHP Superglobals Management                            |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Jiapeng Fan <f2203859@gmail.com>                             |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "SAPI.h"

#include "main/php_streams.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_interfaces.h"
#include "ext/standard/file.h"
#include "php_sg.h"

static HashTable sg_map;

#define SG_DEF_FNAME "trim"

ZEND_DECLARE_MODULE_GLOBALS(sg)

zend_class_entry *sg_ce;

#define SG_CHECK_ENABLE() /* {{{ */ \
{\
    if (!SG_G(enable)) { \
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Please set sg.enable = 1 in php.ini"); \
        RETURN_FALSE; \
    } \
}/* }}} */ \

#define SG_NEW_KEY_EFREE() /* {{{ */ \
{\
    if (key_len != new_key_len) { \
        efree(new_key); \
        new_key = NULL; \
    } \
}/* }}} */ \

#define SG_ZVAL_PSTRING(z, s) do { /* {{{ */ \
        zval *__z = (z);            \
        Z_STRLEN_P(z) = strlen(s);  \
        Z_STRVAL_P(z) = zend_strndup(s, Z_STRLEN_P(z)); \
        Z_TYPE_P(z) = IS_STRING;    \
        Z_SET_REFCOUNT_P(z, 0);     \
        Z_UNSET_ISREF_P(z);         \
    } while(0) /* }}} */ 

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(sg_get_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, default_value)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(sg_getraw_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, default_value)
    ZEND_ARG_INFO(0, maxlen)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(sg_getcache_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, default_value)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(sg_has_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(sg_set_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(sg_del_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("sg.enable", "0", PHP_INI_SYSTEM, OnUpdateBool, enable, zend_sg_globals, sg_globals)
    STD_PHP_INI_ENTRY("sg.global_level", "1", PHP_INI_SYSTEM, OnUpdateBool, global_level, zend_sg_globals, sg_globals)
    STD_PHP_INI_ENTRY("sg.func_name", SG_DEF_FNAME, PHP_INI_ALL, OnUpdateString, func_name, zend_sg_globals, sg_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_sg_init_globals
 */
static void php_sg_init_globals(zend_sg_globals *sg_globals)
{
    memset(sg_globals, 0, sizeof(*sg_globals));
}
/* }}} */

static int _sg_del_cache(char *key, size_t key_len TSRMLS_DC) /* {{{ */
{
    if (SG_G(get_cache) && zend_hash_num_elements(SG_G(get_cache))) {
#if PHP_VERSION_ID >= 70000
        return zend_hash_str_del(SG_G(get_cache), key, key_len);
#else
        return zend_symtable_del(SG_G(get_cache), key, key_len + 1);
#endif
    }
    return FAILURE;
}
/* }}} */

static void _sg_zval_dtor(zval *pvalue) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zend_string_release(Z_STR_P(pvalue));
#else
    free(Z_STRVAL_P(pvalue));
#endif
}
/* }}} */

static size_t _sg_str_convert_self(char *key, size_t key_len, char **new_key TSRMLS_DC) /* {{{ */
{
    size_t need_key_len = key_len;
    const char *p = (key + 1);
    zend_bool is_find = 0;

    if (*p == '.' || key_len == 1) {
        zval *pmap = NULL;
        char tmp[] = {*key, '\0'};

#if PHP_VERSION_ID >= 70000
        if ((pmap = zend_hash_str_find(&sg_map, tmp, sizeof(tmp) - 1)) != NULL) {
#else
        if (zend_hash_find(&sg_map, tmp, sizeof(tmp), (void **) &pmap) != FAILURE) {
#endif
            need_key_len = Z_STRLEN_P(pmap) + (key_len - 1);
            char *skey = (char *)emalloc(need_key_len + 1);
            memcpy(skey, Z_STRVAL_P(pmap), Z_STRLEN_P(pmap));
            memcpy(skey + Z_STRLEN_P(pmap), p, (key_len - 1));
            skey[need_key_len] = '\0';
            *new_key = skey;
            is_find = 1;
        }
    }

    if (!is_find) {
        *new_key = key;
    }

    return need_key_len;
}
/* }}} */

static void _sg_trim_data(zval *pzval TSRMLS_DC) /* {{{ */
{
    if (Z_TYPE_P(pzval) == IS_STRING) {
#if PHP_VERSION_ID >= 70000
        zend_string *sval = php_trim(Z_STR_P(pzval), NULL, 0, 3);
        zval_dtor(pzval);
        ZVAL_STR(pzval, sval);
#else
        char *tmp = php_trim(Z_STRVAL_P(pzval), Z_STRLEN_P(pzval), NULL, 0, NULL, 3 TSRMLS_CC);
        zval_dtor(pzval);
        ZVAL_STRING(pzval, tmp, 0);
#endif
    }
}
/* }}} */

static int _sg_change_callable(zval *pzval TSRMLS_DC) /* {{{ */
{
    int ret = FAILURE;

    if (SG_G(func_name)[0]) {
        zval retval, fname;
        char *method = NULL;
#if PHP_VERSION_ID >= 70000
        zend_string *name = NULL;
        ZVAL_STRING(&fname, SG_G(func_name));
#else
        char *name = NULL;
        ZVAL_STRING(&fname, SG_G(func_name), 0);
#endif
        if (!zend_is_callable(&fname, 0, &name TSRMLS_CC)) {
#if PHP_VERSION_ID >= 70000
            method = ZSTR_VAL(name);
#else
            method = name;
#endif
            zend_error(E_WARNING, "%s() expects the argument (%s) to be a valid callback",
                    get_active_function_name(TSRMLS_C), method ? method : "unknown");
#if PHP_VERSION_ID >= 70000
            zend_string_release(name);
            zval_ptr_dtor(&fname);
#else
            efree(name);
#endif
            return FAILURE;
        }

#if PHP_VERSION_ID >= 70000
        zend_string_release(name);
#else
        efree(name);
#endif

        /* Case sensitive */
        if (strncmp(SG_G(func_name), SG_DEF_FNAME, 4) == 0) {
#if PHP_VERSION_ID >= 70000
            zval_ptr_dtor(&fname);
#endif
            (void)_sg_trim_data(pzval TSRMLS_CC);
            return SUCCESS;
        }

        /* Callable */
#if PHP_VERSION_ID >= 70000
        if ((ret = call_user_function_ex(CG(function_table), NULL, &fname, &retval, 1, pzval, 1, NULL TSRMLS_CC)) == SUCCESS) {
            zval_dtor(pzval);
            ZVAL_COPY_VALUE(pzval, &retval);
        }
        zval_ptr_dtor(&fname);
#else
        if ((ret = call_user_function(EG(function_table), NULL, &fname, &retval, 1, &pzval TSRMLS_CC)) == SUCCESS) {
            zval_dtor(pzval);
            ZVAL_COPY_VALUE(pzval, &retval);
        }
#endif
    }

    return ret;
}
/* }}} */

static zval *sg_strtok_get(char *key, size_t key_len TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zval *pzval = NULL;
#else
    zval **pzval = NULL;
#endif
    HashTable *ht = SG_G(global_http);

    if (strchr(key, '.')) {
        char *seg = NULL, *entry = NULL, *ptr = NULL;

        entry = estrndup(key, key_len);
        if ((seg = php_strtok_r(entry, ".", &ptr))) {
            do {
#if PHP_VERSION_ID >= 70000
                if ((pzval = zend_symtable_str_find(ht, seg, strlen(seg))) == NULL) {
                    efree(entry);
                    return NULL;
                }
                if (Z_ISREF_P(pzval)) {
                    pzval = Z_REFVAL_P(pzval);
                }
                if (Z_TYPE_P(pzval) == IS_ARRAY) {
                    ht = Z_ARRVAL_P(pzval);
                } else {
                    break;
                }
#else
                if (zend_symtable_find(ht, seg, strlen(seg) + 1, (void **) &pzval) == FAILURE) {
                    efree(entry);
                    return NULL;
                }
                if (Z_TYPE_PP(pzval) == IS_ARRAY) {
                    ht = Z_ARRVAL_PP(pzval);
                } else {
                    break;
                }
#endif
            } while ((seg = php_strtok_r(NULL, ".", &ptr)));
        }
        efree(entry);
    } else {
#if PHP_VERSION_ID >= 70000
        pzval = zend_symtable_str_find(ht, key, key_len);
#else
        if (zend_symtable_find(ht, key, key_len + 1, (void **) &pzval) == FAILURE) {
            return NULL;
        }
#endif
    }

#if PHP_VERSION_ID >= 70000
	if (pzval && Z_TYPE_P(pzval) == IS_INDIRECT) {
		pzval = Z_INDIRECT_P(pzval);
	}
    return pzval;
#else
    return *pzval;
#endif
}
/* }}} */

static int sg_strtok_set(char *key, size_t key_len, zval *value TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zval *pzval = NULL;
#else
    zval **pzval = NULL;
#endif
    HashTable *ht = SG_G(global_http);
    int ret = FAILURE;

    if (strchr(key, '.')) {
        char *seg = NULL, *entry = NULL, *ptr = NULL;
#if PHP_VERSION_ID >= 70000
        zval zarr;
#else
        zval *zarr;
#endif
        entry = estrndup(key, key_len);
        if ((seg = php_strtok_r(entry, ".", &ptr))) {
            do {
                if (!(*ptr)) {
#if PHP_VERSION_ID >= 70000
                    if(zend_symtable_str_update(ht, seg, strlen(seg), value)) {
                        ret = SUCCESS;
                    }
#else
                    ret = zend_symtable_update(ht, seg, strlen(seg) + 1, &value, sizeof(value), NULL);
#endif
                    break;
                }

#if PHP_VERSION_ID >= 70000
                if ((pzval = zend_symtable_str_find(ht, seg, strlen(seg)))) {
                    if (Z_ISREF_P(pzval)) {
                        pzval = Z_REFVAL_P(pzval);
                    }
                    if (Z_TYPE_P(pzval) == IS_ARRAY) {
                        ht = Z_ARRVAL_P(pzval);
#if PHP_VERSION_ID >= 70300
                        HT_FLAGS(ht) |= HASH_FLAG_ALLOW_COW_VIOLATION;
#endif
                    } else {
                        goto NEW_ARR;
                    }
                } else {
NEW_ARR:
                    array_init(&zarr);
                    if(!zend_symtable_str_update(ht, seg, strlen(seg), &zarr)) {
                        break;
                    }
                    ht = Z_ARRVAL(zarr);
                }
#else
                if (zend_symtable_find(ht, seg, strlen(seg) + 1, (void **) &pzval) == SUCCESS) {
                    if (Z_TYPE_PP(pzval) == IS_ARRAY) {
                        ht = Z_ARRVAL_PP(pzval);
                    } else {
                        goto NEW_ARR;
                    }
                } else {
NEW_ARR:
                    MAKE_STD_ZVAL(zarr);
                    array_init(zarr);
                    ret = zend_symtable_update(ht, seg, strlen(seg) + 1, &zarr, sizeof(zarr), NULL);
                    ht = Z_ARRVAL_P(zarr);
                }
#endif
                seg = php_strtok_r(NULL, ".", &ptr);
            } while (seg);
            efree(entry);
        }
    } else {
#if PHP_VERSION_ID >= 70000
        if(zend_symtable_str_update(ht, key, key_len, value)) {
            ret = SUCCESS;
        }
#else
        ret = zend_symtable_update(ht, key, key_len + 1, &value, sizeof(value), NULL);
#endif
    }

#if PHP_VERSION_ID >= 70000
    Z_TRY_ADDREF_P(value);
#else
    Z_ADDREF_P(value);
#endif

    return ret;
}
/* }}} */

static int sg_strtok_del(char *key, size_t key_len TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zval *pzval = NULL;
#else
    zval **pzval = NULL;
#endif
    HashTable *ht = SG_G(global_http);
    int ret = FAILURE;

    if (strchr(key, '.')) {
        char *seg = NULL, *entry = NULL, *ptr = NULL;

        entry = estrndup(key, key_len);
        if ((seg = php_strtok_r(entry, ".", &ptr))) {
            do {
#if PHP_VERSION_ID >= 70000
                if (!(*ptr)) {
                    ret = zend_symtable_str_del(ht, seg, strlen(seg));
                    break;
                }
                pzval = zend_symtable_str_find(ht, seg, strlen(seg));
                if (pzval == NULL) {
                    break;
                }
                if (Z_ISREF_P(pzval)) {
                    pzval = Z_REFVAL_P(pzval);
                }
                ht = Z_ARRVAL_P(pzval);
#else
                if (!(*ptr)) {
                    ret = zend_symtable_del(ht, seg, strlen(seg) + 1);
                    break;
                }
                if (zend_symtable_find(ht, seg, strlen(seg) + 1, (void **) &pzval) == FAILURE) {
                    break;
                }
                ht = Z_ARRVAL_PP(pzval);
#endif
                seg = php_strtok_r(NULL, ".", &ptr);
            } while (seg);
            efree(entry);
            return ret;
        }
    }

#if PHP_VERSION_ID >= 70000
    return zend_symtable_str_del(ht, key, key_len);
#else
    return zend_symtable_del(ht, key, key_len + 1);
#endif
}
/* }}} */

static void sg_get_common(char *key, size_t key_len, zval **return_value, zval *default_value, zend_bool is_cache TSRMLS_DC) /* {{{ */
{
    zval *retval = *return_value;

    /* Find a cache data by key */
    if (is_cache && SG_G(func_name)[0] && SG_G(get_cache)) {
#if PHP_VERSION_ID >= 70000
        zval *find = NULL;
        if ((find = zend_hash_str_find(SG_G(get_cache), key, key_len))) {
            ZVAL_COPY(retval, find);
            return;
        }
#else
        zval **find = NULL;
        if (zend_symtable_find(SG_G(get_cache), key, key_len + 1, (void **) &find) == SUCCESS) {
            ZVAL_ZVAL(retval, *find, 1, 0);
            return;
        }
#endif
    }

    char *new_key = NULL;
    size_t new_key_len = _sg_str_convert_self(key, key_len, &new_key TSRMLS_CC);
    zval *pzval = sg_strtok_get(new_key, new_key_len TSRMLS_CC);
    SG_NEW_KEY_EFREE();

    if (pzval) {
#if PHP_VERSION_ID >= 70000
        ZVAL_COPY(retval, pzval);
#else
        ZVAL_ZVAL(retval, pzval, 1, 0);
#endif
        int ret = _sg_change_callable(retval TSRMLS_CC);

        /* Insert Cache */
        if (is_cache && SG_G(func_name)[0] && ret == SUCCESS) {
            if (!SG_G(get_cache)) {
                ALLOC_HASHTABLE(SG_G(get_cache));
                zend_hash_init(SG_G(get_cache), 8, NULL, ZVAL_PTR_DTOR, 0);
            }
#if PHP_VERSION_ID >= 70000
            Z_TRY_ADDREF_P(retval);
            zend_hash_str_add(SG_G(get_cache), key, key_len, retval);
#else
            zval *reg = NULL;
            MAKE_STD_ZVAL(reg);
            *reg = *retval;
            zval_copy_ctor(reg);
            zend_hash_add(SG_G(get_cache), key, key_len + 1, (void **) &reg, sizeof(zval *), NULL);
#endif
        }
        return;
    }

    if (default_value) {
#if PHP_VERSION_ID >= 70000
        ZVAL_COPY(retval, default_value);
#else
        ZVAL_ZVAL(retval, default_value, 1, 0);
#endif
    } else {
        ZVAL_NULL(retval);
    }
}
/* }}} */

/** {{{ proto mixed Sg::get(string $key [, mixed $default_value = null])
 */
static PHP_METHOD(sg, get)
{
    SG_CHECK_ENABLE();

    zval *default_value = NULL;
    char *key = NULL;
    size_t key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &key_len, &default_value) == FAILURE) {
        return;
    }

    (void)sg_get_common(key, key_len, &return_value, default_value, 0 TSRMLS_CC);
}
/* }}} */

/** {{{ proto mixed Sg::getCache(string $key [, mixed $default_value = null])
 */
static PHP_METHOD(sg, getCache)
{
    SG_CHECK_ENABLE();

    zval *default_value = NULL;
    char *key = NULL;
    size_t key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &key_len, &default_value) == FAILURE) {
        return;
    }

    (void)sg_get_common(key, key_len, &return_value, default_value, 1 TSRMLS_CC);
}
/* }}} */

static php_stream *_sg_stream_open_wrapper_ex(char *path, int persistent STREAMS_DC TSRMLS_DC) /* {{{ */
{
    php_stream_wrapper *wrapper = NULL, **wrapperpp = NULL;
    php_stream *stream = NULL;
    php_stream_context *context = NULL;
    char *copy_of_path = NULL;

    context = FG(default_context) ? FG(default_context) : (FG(default_context) = php_stream_context_alloc(TSRMLS_C));
    HashTable *wrapper_hash = _php_stream_get_url_stream_wrappers_hash(TSRMLS_C);

#if PHP_VERSION_ID >= 70000
    if (NULL == (wrapper = zend_hash_str_find_ptr(wrapper_hash, path, 3))) {
        return NULL;
    }
#else
    char *tmp = estrndup(path, 3);
    if (FAILURE == zend_hash_find(wrapper_hash, tmp, 4, (void**)&wrapperpp)) {
        efree(tmp);
        return NULL;
    }
    efree(tmp);
    wrapper = *wrapperpp;
#endif

    if (wrapper) {
        if (!wrapper->wops->stream_opener) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", "wrapper does not support stream open");
        } else {
            stream = wrapper->wops->stream_opener(wrapper, path, "rb", 0,
                    NULL, context STREAMS_REL_CC TSRMLS_CC);
        }
    }

    if (stream) {
        stream->wrapper = wrapper;
        if (stream->orig_path) {
            pefree(stream->orig_path, persistent);
        }
        copy_of_path = pestrdup(path, persistent);
        stream->orig_path = copy_of_path;

#if ZEND_DEBUG
        stream->open_filename = __zend_orig_filename ? __zend_orig_filename : __zend_filename;
        stream->open_lineno = __zend_orig_lineno ? __zend_orig_lineno : __zend_lineno;
#endif
    }

#if PHP_VERSION_ID >= 50500
    if (wrapper && FG(wrapper_errors)) {
#if PHP_VERSION_ID >= 70000
        zend_hash_str_del(FG(wrapper_errors), (const char*)&wrapper, sizeof(wrapper));
#else
        zend_hash_del(FG(wrapper_errors), (const char*)&wrapper, sizeof wrapper);
#endif
        wrapper = NULL;
    }
#else
    if (wrapper) {
        int i;

        for (i = 0; i < wrapper->err_count; i++) {
            efree(wrapper->err_stack[i]);
        }
        if (wrapper->err_stack) {
            efree(wrapper->err_stack);
        }
        wrapper->err_stack = NULL;
        wrapper->err_count = 0;
    }
#endif

    return stream;
}
/* }}} */

/** {{{ proto mixed Sg::getRaw([mixed $default_value = null [, int $maxlen]])
 */
static PHP_METHOD(sg, getRaw)
{
    SG_CHECK_ENABLE();

    zval *default_value = NULL;
#if PHP_VERSION_ID >=  70000
	zend_long maxlen = (ssize_t) PHP_STREAM_COPY_ALL;
#else
	long maxlen = PHP_STREAM_COPY_ALL;
#endif

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zl", &default_value, &maxlen) == FAILURE) {
        return;
    }

    char *path = "php://input";
    int persistent = REPORT_ERRORS & STREAM_OPEN_PERSISTENT;

    php_stream *stream = _sg_stream_open_wrapper_ex(path, persistent STREAMS_CC TSRMLS_CC);
    if (!stream) {
        RETURN_FALSE;
    }

    if (maxlen > INT_MAX) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "maxlen truncated from %pd to %d bytes", maxlen, INT_MAX);
        maxlen = INT_MAX;
    }

    ZVAL_NULL(return_value);

#if PHP_VERSION_ID >= 70000
    zend_string *contents = NULL;
    if ((contents = php_stream_copy_to_mem(stream, maxlen, 0)) != NULL) {
        ZVAL_STR(return_value, contents);
    }
#else
    char *contents = NULL;
    int len = 0;

    if ((len = php_stream_copy_to_mem(stream, &contents, maxlen, 0)) > 0) {
        RETVAL_STRINGL(contents, len, 0);
    }
#endif
    
    if (Z_TYPE_P(return_value) != IS_NULL) {
        (void)_sg_change_callable(return_value TSRMLS_CC);
    } else if (default_value) {
#if PHP_VERSION_ID >= 70000
        ZVAL_COPY(return_value, default_value);
#else
        RETVAL_ZVAL(default_value, 1, 0);
#endif
    } else {
        ZVAL_EMPTY_STRING(return_value);
    }

	php_stream_close(stream);
}
/* }}} */

/** {{{ proto bool Sg::set(string $key, mixed $value)
 */
static PHP_METHOD(sg, set)
{
    SG_CHECK_ENABLE();

    zval *value = NULL;
    char *key = NULL, *new_key = NULL;
    size_t key_len = 0, new_key_len = 0, ret = 0;
   
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &key_len, &value) == FAILURE) {
        return;
    }

    _sg_del_cache(key, key_len TSRMLS_CC);

    new_key_len = _sg_str_convert_self(key, key_len, &new_key TSRMLS_CC);
    ret = sg_strtok_set(new_key, new_key_len, value TSRMLS_CC);
    SG_NEW_KEY_EFREE();

    if (ret == SUCCESS) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}
/* }}} */

/** {{{ proto bool Sg::has(string $key)
 */
static PHP_METHOD(sg, has)
{
    SG_CHECK_ENABLE();

    char *key = NULL, *new_key = NULL;
    size_t key_len = 0, new_key_len = 0;
    zval *pvalue = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

    new_key_len = _sg_str_convert_self(key, key_len, &new_key TSRMLS_CC);
    pvalue = sg_strtok_get(new_key, new_key_len TSRMLS_CC);
    SG_NEW_KEY_EFREE();

    RETURN_BOOL(pvalue);
}
/* }}} */

/** {{{ proto bool Sg::del(string $key [, mixed $... ])
 */
static PHP_METHOD(sg, del)
{
    SG_CHECK_ENABLE();

#if PHP_VERSION_ID >= 70000
    zval *args = NULL;
    zend_string *skey = NULL;
#else
    zval ***args = NULL;
#endif
    size_t key_len = 0, new_key_len = 0;
    char *key = NULL, *new_key = NULL;
    int argc = 0, i = 0, ret = SUCCESS;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &argc) == FAILURE) {
        return;
    }

    for (; i < argc; i++) {
#if PHP_VERSION_ID >= 70000
        skey = zval_get_string(&args[i]);
        key = ZSTR_VAL(skey);
        key_len = ZSTR_LEN(skey);
#else
        convert_to_string_ex(args[i]);
        key = Z_STRVAL_PP(args[i]);
        key_len = Z_STRLEN_PP(args[i]);
#endif
        _sg_del_cache(key, key_len TSRMLS_CC);

        new_key_len = _sg_str_convert_self(key, key_len, &new_key TSRMLS_CC);
        ret = sg_strtok_del(new_key, new_key_len TSRMLS_CC);
        SG_NEW_KEY_EFREE();
#if PHP_VERSION_ID >= 70000
        zend_string_release(skey);
#endif
    }

#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif

    if (argc == 1) {
        if (ret == SUCCESS) {
            RETURN_TRUE;
        } else {
            RETURN_FALSE;
        }
    }

    RETURN_TRUE;
}
/* }}} */

/** {{{ proto bool Sg::all(void)
 */
static PHP_METHOD(sg, all)
{
    SG_CHECK_ENABLE();

#if PHP_VERSION_ID >= 70000
    ZVAL_ARR(return_value, SG_G(global_http));
    Z_TRY_ADDREF_P(return_value);
#else
    Z_TYPE_P(return_value) = IS_ARRAY;
    Z_ARRVAL_P(return_value) = SG_G(global_http);
#endif
}
/* }}} */

/** {{{ proto bool Sg::getCacheAll(void)
 */
static PHP_METHOD(sg, getCacheAll)
{
    SG_CHECK_ENABLE();

    if (SG_G(get_cache)) {
#if PHP_VERSION_ID >= 70000
        ZVAL_ARR(return_value, SG_G(get_cache));
        Z_TRY_ADDREF_P(return_value);
#else
        zval *tmp = NULL;
        array_init(return_value);
        zend_hash_copy(Z_ARRVAL_P(return_value), SG_G(get_cache), (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
#endif
    } else {
        RETURN_NULL();
    }
}
/* }}} */

/** {{{ proto bool Sg::version(void)
 */
static PHP_METHOD(sg, version)
{
    SG_CHECK_ENABLE();

#if PHP_VERSION_ID >= 70000
    RETURN_STRINGL(PHP_SG_VERSION, strlen(PHP_SG_VERSION));
#else
    RETURN_STRINGL(PHP_SG_VERSION, strlen(PHP_SG_VERSION), 1);
#endif
}
/* }}} */

/* {{{ sg_methods[]
 */
static const zend_function_entry sg_methods[] = {
    PHP_ME(sg, get, sg_get_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, getCache, sg_getcache_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, getCacheAll, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, getRaw, sg_getraw_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, set, sg_set_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, has, sg_has_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, del, sg_del_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, all, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, version, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#if PHP_VERSION_ID >= 54000
    PHP_FE_END
#else
    { NULL, NULL, NULL, 0, 0 }
#endif
};
/* }}} */

#if PHP_VERSION_ID >= 70000
static int php7_sg_bind_globals_handler(zend_execute_data *execute_data TSRMLS_DC) /* {{{ */
{
    const zend_op *opline = execute_data->opline;
    zval *var = NULL, *value = NULL, *pzval = NULL;
    uint32_t idx = 0;
    char *key = NULL;
    size_t key_len = 0;

    do {
#if PHP_VERSION_ID >= 70300
        var = RT_CONSTANT(opline, opline->op2);
#else
        var = EX_CONSTANT(opline->op2);
#endif 
        key = Z_STRVAL_P(var);
        key_len = Z_STRLEN_P(var);

        if ((key_len > 2 && *(key + 1) == '_') || (key_len == 1)) {
            char *p = NULL, *n_key = NULL, *new_key = NULL;
            size_t new_key_len = 0;
           
            /* If the value is found in execute_data, continue */
            zval *tmp_value = EX_VAR(opline->op1.var);
            if (tmp_value && Z_TYPE_P(tmp_value) != IS_UNDEF) {
                continue;
            }

            p = estrndup(key, key_len);
            n_key = p;

            /* Start with p++ */
            p++;
            /* global $g, $p, $c, $g_key, $p_key, $c_key, ... */
            while(*p != '\0') {
                if (*p == '_') {
                    *p = '.';
                    if (EXPECTED(SG_G(global_level))) {
                        break;
                    }
                }
                p++;
            }

            new_key_len = _sg_str_convert_self(n_key, key_len, &new_key TSRMLS_CC);
            /* The Key Not found in SG_MAP, new_key address same as n_key */
            if (new_key_len == key_len) {
                efree(n_key);
                continue;
            }

            value = zend_hash_find(&EG(symbol_table), Z_STR_P(var));

            if (value == NULL) {
                value = zend_hash_add_new(&EG(symbol_table), Z_STR_P(var), &EG(uninitialized_zval));
                idx = ((char*)value - (char*)EG(symbol_table).arData) / sizeof(Bucket);
                CACHE_PTR(Z_CACHE_SLOT_P(var), (void*)(uintptr_t)(idx + 1));
                goto ADD_SYMBOL_VAR;
            } else if (Z_TYPE_P(value) == IS_INDIRECT) {
                value = Z_INDIRECT_P(value);
                if (UNEXPECTED(Z_TYPE_P(value) == IS_UNDEF)) {
ADD_SYMBOL_VAR:
                    pzval = sg_strtok_get(new_key, new_key_len TSRMLS_CC);
                    if (pzval) {
                        zend_reference *ref = NULL;
                        (void)_sg_change_callable(pzval TSRMLS_CC);
                        if (UNEXPECTED(!Z_ISREF_P(pzval))) {
#if PHP_VERSION_ID >= 70300
                            ZVAL_MAKE_REF_EX(pzval, 2);
                            ref = Z_REF_P(pzval);
                            ZVAL_REF(value, ref);
#else
                            ZVAL_NEW_REF(pzval, pzval);
                            ref = Z_REF_P(pzval);
                            GC_REFCOUNT(ref)++;
                            ZVAL_REF(value, ref);
#endif
                        } else {
#if PHP_VERSION_ID >= 70300
                            ref = Z_REF_P(pzval);
                            GC_ADDREF(ref);
                            ZVAL_COPY_VALUE(value, pzval);
#else
                            GC_REFCOUNT(Z_REF_P(pzval))++;
                            ZVAL_COPY_VALUE(value, pzval);
#endif
                        }
                    }
                }
            }
            efree(n_key);
            efree(new_key);
        }
    } while (UNEXPECTED((++opline)->opcode == ZEND_BIND_GLOBAL));

    return ZEND_USER_OPCODE_DISPATCH;
}
/* }}} */
#else
static int php5_sg_bind_globals_handler(zend_execute_data *execute_data TSRMLS_DC) /* {{{ */
{
    do {
        const zend_op *opline = execute_data->opline;
        zend_uchar op1_type = opline->op1_type;

        if (op1_type != IS_CV) {
            break;
        }

        const char *key = NULL;
        int key_len = 0;

        zend_compiled_variable *cv = &EG(active_op_array)->vars[opline->op1.var];

        key = cv->name;
        key_len = cv->name_len;

        if ((key_len > 2 && *(key + 1) == '_') || (key_len == 1)) {
            char *p = NULL, *n_key = NULL, *new_key = NULL;
            size_t new_key_len = 0;
            zval **value = NULL, *pzval = NULL;

            p = estrndup(key, key_len);
            n_key = p;

            p++;
            while(*p != '\0') {
                if (*p == '_') {
                    *p = '.';
                    if (EXPECTED(SG_G(global_level))) {
                        break;
                    }
                }
                p++;
            }

            new_key_len = _sg_str_convert_self(n_key, key_len, &new_key TSRMLS_CC);
            if (new_key_len == key_len) {
                efree(n_key);
                break;
            }

            if (zend_hash_quick_find(&EG(symbol_table), key, key_len + 1, cv->hash_value, (void **) &value) == SUCCESS
                    && (Z_TYPE_PP(value) == IS_NULL)) {
                pzval = sg_strtok_get(new_key, new_key_len TSRMLS_CC);
                if (pzval) {
                    (void)_sg_change_callable(pzval TSRMLS_CC);
                    if (UNEXPECTED(!Z_ISREF_P(pzval))) {
                        Z_SET_ISREF_P(pzval);
                    }
                    *value = pzval;
                    Z_ADDREF_P(pzval);
                }
            }

            efree(n_key);
            efree(new_key);
        }
    } while (0);

    return ZEND_USER_OPCODE_DISPATCH;
}
/* }}} */
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
static PHP_MINIT_FUNCTION(sg)
{
    ZEND_INIT_MODULE_GLOBALS(sg, php_sg_init_globals, NULL);
    REGISTER_INI_ENTRIES();

    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Sg", sg_methods);
#if PHP_VERSION_ID >= 70000
    sg_ce = zend_register_internal_class_ex(&ce, NULL);
#else
    sg_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
#endif

    if (SG_G(enable)) {
        zval preg; int i = 0;
        const char *k[] = {"g", "p", "c", "s", "f", "n", "r", "e"};
        const char *v[] = {"_GET", "_POST", "_COOKIE", "_SERVER", "_FILES", "_SESSION", "_REQUEST", "_ENV"};
        zend_hash_init(&sg_map, 16, NULL, (dtor_func_t) _sg_zval_dtor, 1);
#if PHP_VERSION_ID >= 70000
        while(i < 8) {
            ZVAL_NEW_STR(&preg, zend_string_init(v[i], strlen(v[i]), 1));
            zend_hash_str_add_new(&sg_map, k[i], strlen(k[i]), &preg);
            i++;
        }
#else
        while(i < 8) {
            SG_ZVAL_PSTRING(&preg, v[i]);
            zend_hash_add(&sg_map, k[i], strlen(k[i]) + 1, (void *)&preg, sizeof(zval), NULL);
            i++;
        }
#endif

#if PHP_VERSION_ID >= 70000
        zend_set_user_opcode_handler(ZEND_BIND_GLOBAL, php7_sg_bind_globals_handler);
#else
        zend_set_user_opcode_handler(ZEND_ASSIGN_REF, php5_sg_bind_globals_handler);
#endif
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
static PHP_MSHUTDOWN_FUNCTION(sg)
{
    if (SG_G(enable)) {
        zend_hash_destroy(&sg_map);
    }

    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
static PHP_RINIT_FUNCTION(sg)
{
    if (SG_G(enable)) {
#if PHP_VERSION_ID >= 70000
        if (PG(auto_globals_jit)) {
            zend_is_auto_global_str(ZEND_STRL("_SERVER"));
            zend_is_auto_global_str(ZEND_STRL("_REQUEST"));
            zend_is_auto_global_str(ZEND_STRL("_ENV"));
        }
#else
        zend_is_auto_global("_SERVER", sizeof("_SERVER") - 1 TSRMLS_CC);
        zend_is_auto_global("_REQUEST", sizeof("_REQUEST") - 1 TSRMLS_CC);
        zend_is_auto_global("_ENV", sizeof("_ENV") - 1 TSRMLS_CC);
#endif
        SG_G(global_http) = &EG(symbol_table);
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
static PHP_RSHUTDOWN_FUNCTION(sg)
{
    if (SG_G(get_cache)) {
        zend_hash_destroy(SG_G(get_cache));
        FREE_HASHTABLE(SG_G(get_cache));
        SG_G(get_cache) = NULL;
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
static PHP_MINFO_FUNCTION(sg)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "sg support", "enabled");
    php_info_print_table_row(2, "Author", PHP_SG_AUTHOR);
    php_info_print_table_row(2, "Version", PHP_SG_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ sg_module_entry
 */
zend_module_entry sg_module_entry = {
    STANDARD_MODULE_HEADER_EX,
    NULL,
    NULL,
    "sg",
    NULL,
    PHP_MINIT(sg),
    PHP_MSHUTDOWN(sg),
    PHP_RINIT(sg),
    PHP_RSHUTDOWN(sg),
    PHP_MINFO(sg),
    PHP_SG_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SG
ZEND_GET_MODULE(sg)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 * vim600: et sw=4 ts=4 fdm=marker
 * vim<600: et sw=4 ts=4
 */
