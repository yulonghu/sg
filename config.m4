dnl $Id$
dnl config.m4 for extension sg

PHP_ARG_ENABLE(sg, whether to enable sg support,
Make sure that the comment is aligned:
[  --enable-sg           Enable sg support])

if test "$PHP_SG" != "no"; then
  PHP_NEW_EXTENSION(sg, sg.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
