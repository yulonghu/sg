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

static HashTable sg_map;

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
    } while(0) /* }}} */ 

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

static void sg_zval_dtor(zval *pvalue) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zend_string_release(Z_STR_P(pvalue));
#else
    free(Z_STRVAL_P(pvalue));
#endif
}
/* }}} */

static size_t sg_str_convert_self(char *key, size_t key_len, char **new_key TSRMLS_DC) /* {{{ */
{
    size_t need_key_len = key_len;
    const char *p = (key + 1);
    zend_bool is_find = 0;

    if (*p == '.' || key_len == 1) {
        zval *pmap = NULL;
#if PHP_VERSION_ID >= 70000
        if ((pmap = zend_hash_str_find(&sg_map, key, 1)) != NULL) {
#else
        if (zend_symtable_find(&sg_map, key, key_len + 1, (void **) &pmap) != FAILURE) {
#endif
            need_key_len = spprintf(new_key, 0, "%s%s", Z_STRVAL_P(pmap), p);
            is_find = 1;
        }
    }

    if (!is_find) {
        *new_key = key;
    }

    return need_key_len;
}
/* }}} */

static zval *sg_strtok_get(char *key, size_t key_len TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zval *pzval = NULL;
#else
    zval **pzval = NULL;
#endif
    HashTable *ht = SG_G(http_globals);

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

static int sg_strtok_set(char *key, size_t key_len, zval *value TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zval *pzval = NULL;
#else
    zval **pzval = NULL;
#endif
    HashTable *ht = SG_G(http_globals);
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

static int sg_strtok_del(char *key, size_t key_len TSRMLS_DC) /* {{{ */
{
#if PHP_VERSION_ID >= 70000
    zval *pzval = NULL;
#else
    zval **pzval = NULL;
#endif
    HashTable *ht = SG_G(http_globals);
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
static PHP_METHOD(sg, get)
{
    SG_CHECK_ENABLE();

    zval *default_value = NULL, *pzval = NULL;
    char *key = NULL, *new_key = NULL;
    size_t key_len = 0, new_key_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &key_len, &default_value) == FAILURE) {
        return;
    }

    new_key_len = sg_str_convert_self(key, key_len, &new_key TSRMLS_CC);
    pzval = sg_strtok_get(new_key, new_key_len TSRMLS_CC);
    SG_NEW_KEY_EFREE();

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
static PHP_METHOD(sg, set)
{
    SG_CHECK_ENABLE();

    zval *value = NULL;
    char *key = NULL, *new_key = NULL;
    size_t key_len = 0, new_key_len = 0, ret = 0;
   
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &key_len, &value) == FAILURE) {
        return;
    }

    new_key_len = sg_str_convert_self(key, key_len, &new_key TSRMLS_CC);
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

    new_key_len = sg_str_convert_self(key, key_len, &new_key TSRMLS_CC);
    pvalue = sg_strtok_get(new_key, new_key_len TSRMLS_CC);
    SG_NEW_KEY_EFREE();

    RETURN_BOOL(pvalue);
}
/* }}} */

/** {{{ proto bool Sg::del(string $key)
 */
static PHP_METHOD(sg, del)
{
    SG_CHECK_ENABLE();

    char *key = NULL, *new_key = NULL;
    size_t key_len = 0, new_key_len = 0, ret = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
        return;
    }

    new_key_len = sg_str_convert_self(key, key_len, &new_key TSRMLS_CC);
    ret = sg_strtok_del(new_key, new_key_len TSRMLS_CC);
    SG_NEW_KEY_EFREE();

    if (ret == SUCCESS) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}
/* }}} */

/** {{{ proto bool Sg::all(void)
 */
static PHP_METHOD(sg, all)
{
    SG_CHECK_ENABLE();

#if PHP_VERSION_ID >= 70000
    ZVAL_ARR(return_value, SG_G(http_globals));
    Z_TRY_ADDREF_P(return_value);
#else
    Z_TYPE_P(return_value) = IS_ARRAY;
    Z_ARRVAL_P(return_value) = SG_G(http_globals);
    Z_ADDREF_P(return_value);
#endif
}
/* }}} */

/* {{{ sg_methods[]
 */
static const zend_function_entry sg_methods[] = {
    PHP_ME(sg, get, sg_get_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, set, sg_set_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, has, sg_has_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, del, sg_del_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(sg, all, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#if PHP_VERSION_ID >= 54000
    PHP_FE_END
#else
    { NULL, NULL, NULL, 0, 0 }
#endif
};
/* }}} */

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
        zend_hash_init(&sg_map, 8, NULL, (dtor_func_t) sg_zval_dtor, 1);
#if PHP_VERSION_ID >= 70000
        zval preg;

        ZVAL_NEW_STR(&preg, zend_string_init(ZEND_STRL("_GET"), 1));
        zend_hash_str_add_new(&sg_map, "g", sizeof("g") - 1, &preg);

        ZVAL_NEW_STR(&preg, zend_string_init(ZEND_STRL("_POST"), 1));
        zend_hash_str_add_new(&sg_map, "p", sizeof("p") - 1, &preg);

        ZVAL_NEW_STR(&preg, zend_string_init(ZEND_STRL("_COOKIE"), 1));
        zend_hash_str_add_new(&sg_map, "c", sizeof("c") - 1, &preg);

        ZVAL_NEW_STR(&preg, zend_string_init(ZEND_STRL("_SERVER"), 1));
        zend_hash_str_add_new(&sg_map, "s", sizeof("s") - 1, &preg);

        ZVAL_NEW_STR(&preg, zend_string_init(ZEND_STRL("_FILES"), 1));
        zend_hash_str_add_new(&sg_map, "f", sizeof("f") - 1, &preg);
#else
        zval preg;

        SG_ZVAL_PSTRING(&preg, "_GET");
        zend_hash_add(&sg_map, "g", sizeof("g"), (void *)&preg, sizeof(zval), NULL);

        SG_ZVAL_PSTRING(&preg, "_POST");
        zend_hash_add(&sg_map, "p", sizeof("p"), (void *)&preg, sizeof(zval), NULL);
        
        SG_ZVAL_PSTRING(&preg, "_COOKIE");
        zend_hash_add(&sg_map, "c", sizeof("c"), (void *)&preg, sizeof(zval), NULL);

        SG_ZVAL_PSTRING(&preg, "_SERVER");
        zend_hash_add(&sg_map, "s", sizeof("s"), (void *)&preg, sizeof(zval), NULL);

        SG_ZVAL_PSTRING(&preg, "_FILES");
        zend_hash_add(&sg_map, "f", sizeof("f"), (void *)&preg, sizeof(zval), NULL);
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
        }
#else
        zend_is_auto_global("_SERVER", sizeof("_SERVER") - 1 TSRMLS_CC);
#endif
        SG_G(http_globals) = &EG(symbol_table);
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
static PHP_RSHUTDOWN_FUNCTION(sg)
{
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
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
