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
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_interfaces.h"
#include "php_sg.h"

ZEND_DECLARE_MODULE_GLOBALS(sg)

#define CHECK_SG_ENABLE() /* {{{ */ \
{\
    if (!SG_G(enable)) { \
        php_error_docref(NULL, E_WARNING, "Please set sg.enable = 1 in php.ini"); \
        RETURN_FALSE; \
    } \
}/* }}} */ \

zend_class_entry *sg_ce;

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(sg_get_arginfo, 0, 0, 1)
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
    STD_PHP_INI_ENTRY("sg.auto_trim", "1", PHP_INI_ALL, OnUpdateBool, auto_trim, zend_sg_globals, sg_globals)
    STD_PHP_INI_ENTRY("sg.enable", "0", PHP_INI_SYSTEM, OnUpdateBool, enable, zend_sg_globals, sg_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_sg_init_globals
 */
static void php_sg_init_globals(zend_sg_globals *sg_globals)
{
    memset(sg_globals, 0, sizeof(*sg_globals));
}
/* }}} */

static zval *sg_strtok_get(char *key, int key_len TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zval *pzval = NULL;
    HashTable *ht = Z_ARRVAL(SG_G(http_globals));
#else
    zval **pzval = NULL;
    HashTable *ht = Z_ARRVAL_P(SG_G(http_globals));
#endif

    if (zend_memrchr(key, '.', key_len)) {
        char *seg = NULL, *entry = NULL, *ptr = NULL;

        entry = estrndup(key, key_len);
        if ((seg = php_strtok_r(entry, ".", &ptr))) {
            do {
                if (ht == NULL) {
                    efree(entry);
                    return NULL;
                }
#if PHP_VERSION_ID >= 70000
                if ((pzval = zend_symtable_str_find(ht, seg, strlen(seg))) == NULL) {
                    efree(entry);
                    return NULL;
                }
                if (Z_TYPE_P(pzval) == IS_ARRAY) {
                    ht = Z_ARRVAL_P(pzval);
                } else {
                    ht = NULL;
                }
#else
                if (zend_symtable_find(ht, seg, strlen(seg) + 1, (void **) &pzval) == FAILURE) {
                    efree(entry);
                    return NULL;
                }
                if (Z_TYPE_PP(pzval) == IS_ARRAY) {
                    ht = Z_ARRVAL_PP(pzval);
                } else {
                    ht = NULL;
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
    return pzval;
#else
    return *pzval;
#endif
}
/* }}} */

static int sg_strtok_set(char *key, int key_len, zval *value TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zval *pzval = NULL;
    HashTable *ht = Z_ARRVAL(SG_G(http_globals));
#else
    zval **pzval = NULL;
    HashTable *ht = Z_ARRVAL_P(SG_G(http_globals));
#endif
    int ret = FAILURE;

    if (zend_memrchr(key, '.', key_len)) {
        char *seg = NULL, *entry = NULL, *ptr = NULL;
#if PHP_VERSION_ID >= 70000
        zval zarr;
#else
        zval *zarr;
#endif
        entry = estrndup(key, key_len);
        if ((seg = php_strtok_r(entry, ".", &ptr))) {
            do {
                if (strlen(ptr) < 1) {
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
                pzval = zend_symtable_str_find(ht, seg, strlen(seg));
                if (!pzval || Z_TYPE_P(pzval) != IS_ARRAY) {
                    array_init(&zarr);
                    if(!zend_symtable_str_update(ht, seg, strlen(seg), &zarr)) {
                        break;
                    }
                    pzval = &zarr;
                }

                ht = Z_ARRVAL_P(pzval);
#else
                if (zend_symtable_find(ht, seg, strlen(seg) + 1, (void **) &pzval) == FAILURE || Z_TYPE_PP(pzval) != IS_ARRAY) {
                    MAKE_STD_ZVAL(zarr);
                    array_init(zarr);
                    ret = zend_symtable_update(ht, seg, strlen(seg) + 1, &zarr, sizeof(zarr), NULL);
                    pzval = &zarr;
                }

                ht = Z_ARRVAL_PP(pzval);
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

static int sg_strtok_del(char *key, int key_len TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zval *pzval = NULL;
    HashTable *ht = Z_ARRVAL(SG_G(http_globals));
#else
    zval **pzval = NULL;
    HashTable *ht = Z_ARRVAL_P(SG_G(http_globals));
#endif
    int ret = FAILURE;

    if (zend_memrchr(key, '.', key_len)) {
        char *last_seg = NULL, *seg = NULL, *entry = NULL, *ptr = NULL;

        entry = estrndup(key, key_len);
        if ((seg = php_strtok_r(entry, ".", &ptr))) {
            do {
#if PHP_VERSION_ID >= 70000
                if (!strlen(ptr)) {
                    ret = zend_symtable_str_del(ht, seg, strlen(seg));
                    break;
                }
                pzval = zend_symtable_str_find(ht, seg, strlen(seg));
                if (pzval == NULL) {
                    break;
                }
                ht = Z_ARRVAL_P(pzval);
#else
                if (!strlen(ptr)) {
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

/** {{{ proto mixed Sg::get(string $key [, mixed $default_value = null])
 */
PHP_METHOD(sg, get)
{
    CHECK_SG_ENABLE();
    zval *default_value = NULL, *pzval = NULL;

#if PHP_VERSION_ID >= 70000
    zend_string *key = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(key)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(default_value)
    ZEND_PARSE_PARAMETERS_END();
    
    pzval = sg_strtok_get(ZSTR_VAL(key), ZSTR_LEN(key) TSRMLS_CC);
#else
    char *key = NULL;
    int key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &key_len, &default_value) == FAILURE) {
        return;
    }

    pzval = sg_strtok_get(key, key_len TSRMLS_CC);
#endif

    if (pzval) {
        if (Z_TYPE_P(pzval) == IS_STRING && SG_G(auto_trim)) {
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
        RETURN_ZVAL(pzval, 1, 0);
    }

    if (default_value) {
        RETURN_ZVAL(default_value, 1, 0);
    }

    RETURN_NULL();
}
/* }}} */

/** {{{ proto bool Sg::set(string $key, mixed $value)
 */
PHP_METHOD(sg, set)
{
    CHECK_SG_ENABLE();
    zval *value = NULL;

#if PHP_VERSION_ID >= 70000
    zend_string *key = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(key)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    if (sg_strtok_set(ZSTR_VAL(key), ZSTR_LEN(key), value TSRMLS_CC) == SUCCESS) {
        RETURN_TRUE;
    }
#else
    char *key = NULL;
    int key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &key_len, &value) == FAILURE) {
        return;
    }

    if (sg_strtok_set(key, key_len, value TSRMLS_CC) == SUCCESS) {
        RETURN_TRUE;
    }
#endif

    RETURN_FALSE;
}
/* }}} */

/** {{{ proto bool Sg::has(string $key)
*/
PHP_METHOD(sg, has)
{
    CHECK_SG_ENABLE();

#if PHP_VERSION_ID >= 70000
    zend_string *key = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    if (sg_strtok_get(ZSTR_VAL(key), ZSTR_LEN(key) TSRMLS_CC) != NULL) {
        RETURN_TRUE;
    }
#else
    char *key = NULL;
    int key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &key_len) == FAILURE) {
        return;
    }

     if(sg_strtok_get(key, key_len TSRMLS_CC) != NULL) {
        RETURN_TRUE;
     }
#endif
    
    RETURN_FALSE;
}
/* }}} */

/** {{{ proto bool Sg::del(string $key)
 */
PHP_METHOD(sg, del)
{
    CHECK_SG_ENABLE();

#if PHP_VERSION_ID >= 70000
    zend_string *key = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    if (sg_strtok_del(ZSTR_VAL(key), ZSTR_LEN(key) TSRMLS_CC) == SUCCESS) {
        RETURN_TRUE;
    }
#else
    char *key = NULL;
    int key_len = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &key_len) == FAILURE) {
        return;
    }

    if (sg_strtok_del(key, key_len TSRMLS_CC) == SUCCESS) {
        RETURN_TRUE;
    }
#endif
    
    RETURN_FALSE;
}
/* }}} */

/* {{{ sg_methods[]
 */
const zend_function_entry sg_methods[] = {
    PHP_ME(sg, get, sg_get_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, set, sg_set_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, has, sg_has_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, del, sg_del_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#if PHP_VERSION_ID >= 54000
    PHP_FE_END
#else
    { NULL, NULL, NULL, 0, 0 }
#endif
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(sg)
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

    if (strcasecmp("cli", sapi_module.name) == 0) {
        SG_G(cli) = 1;
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(sg)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(sg)
{
#if defined(COMPILE_DL_SG) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

    if (SG_G(enable)) {
#if PHP_VERSION_ID >= 70000
        array_init(&SG_G(http_globals));
#else
        SG_G(http_globals) = NULL;
        MAKE_STD_ZVAL(SG_G(http_globals));
        array_init(SG_G(http_globals));
#endif
        if (!SG_G(cli)) {
#if PHP_VERSION_ID >= 70000
            HashTable *ht = Z_ARRVAL(SG_G(http_globals));

            zval *get = &PG(http_globals)[TRACK_VARS_GET];
            zval *post = &PG(http_globals)[TRACK_VARS_POST];
            zval *cookie = &PG(http_globals)[TRACK_VARS_COOKIE];

            if (PG(auto_globals_jit)) {
                zend_is_auto_global_str(ZEND_STRL("_SERVER"));
            }
            zval *server = &PG(http_globals)[TRACK_VARS_SERVER];
            zval *files = &PG(http_globals)[TRACK_VARS_FILES];

            zend_hash_str_add_new(ht, "g", 1, get);
            zend_hash_str_add_new(ht, "p", 1, post);
            zend_hash_str_add_new(ht, "c", 1, cookie);

            zend_hash_str_add_new(ht, "s", 1, server);
            zend_hash_str_add_new(ht, "f", 1, files);

            Z_TRY_ADDREF_P(get);
            Z_TRY_ADDREF_P(post);
            Z_TRY_ADDREF_P(cookie);
            Z_TRY_ADDREF_P(server);
            Z_TRY_ADDREF_P(files);
#else
            zend_is_auto_global("_SERVER", sizeof("_SERVER") - 1 TSRMLS_CC);
            zval *ht = SG_G(http_globals);

            zval *get = PG(http_globals)[TRACK_VARS_GET];
            zval *post = PG(http_globals)[TRACK_VARS_POST];
            zval *cookie = PG(http_globals)[TRACK_VARS_COOKIE];
            zval *server = PG(http_globals)[TRACK_VARS_SERVER];
            zval *files = PG(http_globals)[TRACK_VARS_FILES];

            add_assoc_zval_ex(ht, "g", sizeof("g"), get);
            add_assoc_zval_ex(ht, "p", sizeof("p"), post);
            add_assoc_zval_ex(ht, "c", sizeof("c"), cookie);
            add_assoc_zval_ex(ht, "s", sizeof("s"), server);
            add_assoc_zval_ex(ht, "f", sizeof("f"), files);

            Z_ADDREF_P(get);
            Z_ADDREF_P(post);
            Z_ADDREF_P(cookie);
            Z_ADDREF_P(server);
            Z_ADDREF_P(files);
#endif
        }
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(sg)
{
    if (SG_G(enable)) {
#if PHP_VERSION_ID >= 70000
        zval_ptr_dtor(&SG_G(http_globals));
        ZVAL_UNDEF(&SG_G(http_globals));
#else
        zval_ptr_dtor(&(SG_G(http_globals)));
        SG_G(http_globals) = NULL;
#endif
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(sg)
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
    STANDARD_MODULE_HEADER,
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
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(sg)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
