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

#ifndef PHP_SG_H
#define PHP_SG_H

extern zend_module_entry sg_module_entry;
#define phpext_sg_ptr &sg_module_entry

#define PHP_SG_VERSION "0.1.0"
#define PHP_SG_AUTHOR  "Jiapeng Fan"

#ifdef PHP_WIN32
#	define PHP_SG_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_SG_API __attribute__ ((visibility("default")))
#else
#	define PHP_SG_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZTS
#define SG_G(v) TSRMG(sg_globals_id, zend_sg_globals *, v) 
#else
#define SG_G(v) (sg_globals.v)
#endif

ZEND_BEGIN_MODULE_GLOBALS(sg)
	zend_bool auto_trim;
	zval http_globals;
	zend_bool enable;
	zend_bool cli;
ZEND_END_MODULE_GLOBALS(sg)

#if defined(ZTS) && defined(COMPILE_DL_SG)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

extern ZEND_DECLARE_MODULE_GLOBALS(sg);

#endif	/* PHP_SG_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
