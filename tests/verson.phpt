--TEST--
Check for sg::version(void)
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php

var_dump(sg::version());

?>
--EXPECT--
string(5) "3.0.1"

