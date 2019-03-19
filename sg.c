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
  | Author: Jiapeng Fan                                                  |
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

#define CHECK_SG_ENABLE() \
{\
	if (!SG_G(enable)) { \
		php_error_docref(NULL, E_WARNING, "Please set sg.enable = 1 in php.ini"); \
		RETURN_FALSE; \
	} \
} \

zend_class_entry *sg_ce;

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("sg.auto_trim", "1", PHP_INI_ALL, OnUpdateBool, auto_trim, zend_sg_globals, sg_globals)
    STD_PHP_INI_ENTRY("sg.enable", "0", PHP_INI_SYSTEM, OnUpdateBool, enable, zend_sg_globals, sg_globals)
PHP_INI_END()
/* }}} */

/* }}} */

/* {{{ php_sg_init_globals
 */
static void php_sg_init_globals(zend_sg_globals *sg_globals)
{
	memset(sg_globals, 0, sizeof(*sg_globals));
}
/* }}} */

static zval *sg_strtok_get(zend_string *key) /* {{{ */
{
	zval *pzval = NULL;
	HashTable *ht = Z_ARRVAL(SG_G(http_globals));

    if (zend_memrchr(ZSTR_VAL(key), '.', ZSTR_LEN(key))) {
        char *seg = NULL, *entry = NULL, *ptr = NULL;

        entry = estrndup(ZSTR_VAL(key), ZSTR_LEN(key));
        if ((seg = php_strtok_r(entry, ".", &ptr))) {
            do {
                if (ht == NULL || (pzval = zend_symtable_str_find(ht, seg, strlen(seg))) == NULL) {
                    efree(entry);
                    return NULL;
                }
                if (Z_TYPE_P(pzval) == IS_ARRAY) {
                    ht = Z_ARRVAL_P(pzval);
                } else {
                    ht = NULL;
                }
            } while ((seg = php_strtok_r(NULL, ".", &ptr)));
        }
        efree(entry);
    } else {
        pzval = zend_symtable_find(ht, key);
    }

    return pzval;
}
/* }}} */

static zval *sg_strtok_set(zend_string *key, zval *value) /* {{{ */
{
    zval *pzval = NULL;
	HashTable *ht = Z_ARRVAL(SG_G(http_globals));

    if (zend_memrchr(ZSTR_VAL(key), '.', ZSTR_LEN(key))) {
        char *seg = NULL, *entry = NULL, *ptr = NULL;
        zval zarr;

        entry = estrndup(ZSTR_VAL(key), ZSTR_LEN(key));
        if ((seg = php_strtok_r(entry, ".", &ptr))) {
            do {
				if (strlen(ptr) < 1) {
					pzval = zend_symtable_str_update(ht, seg, strlen(seg), value);
					break;
				}
                pzval = zend_symtable_str_find(ht, seg, strlen(seg));
				if (!pzval || Z_TYPE_P(pzval) != IS_ARRAY) {
					array_init(&zarr);
					zend_symtable_str_update(ht, seg, strlen(seg), &zarr);
					pzval = &zarr;
				}

                ht = Z_ARRVAL_P(pzval);
                seg = php_strtok_r(NULL, ".", &ptr);
            } while (seg);
            efree(entry);
        }
    } else {
        pzval = zend_symtable_update(ht, key, value);
    }

    Z_TRY_ADDREF_P(value);

	return pzval;
}/*}}}*/

static int sg_strtok_del(zend_string *key) /* {{{ */
{
    zval *pzval = NULL;
	HashTable *ht = Z_ARRVAL(SG_G(http_globals));
	int ret = FAILURE;

	if (zend_memrchr(ZSTR_VAL(key), '.', ZSTR_LEN(key))) {
		char *last_seg = NULL, *seg = NULL, *entry = NULL, *ptr = NULL;

		entry = estrndup(ZSTR_VAL(key), ZSTR_LEN(key));
		if ((seg = php_strtok_r(entry, ".", &ptr))) {
			do {
				if (!strlen(ptr)) {
					ret = zend_symtable_str_del(ht, seg, strlen(seg));
					break;
				}
				pzval = zend_symtable_str_find(ht, seg, strlen(seg));
				if (pzval == NULL) {
					break;
				}
				ht = Z_ARRVAL_P(pzval);
				seg = php_strtok_r(NULL, ".", &ptr);
			} while (seg);
			efree(entry);
			return ret;
		}
	}

	return zend_symtable_del(ht, key);
}/*}}}*/

/** {{{ proto mixed Sg::get(string $key [, mixed $default_value = null])
*/
PHP_METHOD(sg, get)
{
	zend_string *key = NULL;
	zval *default_value = NULL, *pzval = NULL;

	CHECK_SG_ENABLE();

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_STR(key)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(default_value)
	ZEND_PARSE_PARAMETERS_END();

	pzval = sg_strtok_get(key);

	if (pzval) {
		if (Z_TYPE_P(pzval) == IS_STRING && SG_G(auto_trim)) {
			zend_string *sval = php_trim(Z_STR_P(pzval), NULL, 0, 3);
			zval_ptr_dtor(pzval);
			ZVAL_STR(pzval, sval);
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
	zend_string *key = NULL;
	zval *value = NULL;

	CHECK_SG_ENABLE();

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(key)
		Z_PARAM_ZVAL(value)
	ZEND_PARSE_PARAMETERS_END();

	if (sg_strtok_set(key, value)) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/** {{{ proto bool Sg::has(string $key)
*/
PHP_METHOD(sg, has)
{
	zend_string *key = NULL;
	
	CHECK_SG_ENABLE();

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(key)
	ZEND_PARSE_PARAMETERS_END();

	if (sg_strtok_get(key) != NULL) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/** {{{ proto bool Sg::del(string $key)
*/
PHP_METHOD(sg, del)
{
	zend_string *key = NULL;

	CHECK_SG_ENABLE();

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(key)
	ZEND_PARSE_PARAMETERS_END();

	if (sg_strtok_del(key) == SUCCESS) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ sg_methods[]
 */
const zend_function_entry sg_methods[] = {
	PHP_ME(sg, get, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(sg, set, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(sg, has, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(sg, del, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
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
	sg_ce = zend_register_internal_class_ex(&ce, NULL);

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
		array_init(&SG_G(http_globals));
		if (!SG_G(cli)) {
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
		zval_ptr_dtor(&SG_G(http_globals));
		ZVAL_UNDEF(&SG_G(http_globals));
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
