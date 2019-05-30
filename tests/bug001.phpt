--TEST--
Check for Z_TYPE_INFO = IS_INDIRECT
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
sg.global_level = 1
--FILE--
<?php

/* lijinguo */

$a = '$a';

sg::set('$a', "a");

echo $GLOBALS[$$a], PHP_EOL;
echo sg::get($$a), PHP_EOL;

?>
--EXPECT--
$a
$a

