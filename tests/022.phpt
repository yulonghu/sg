--TEST--
Check for sg::getRaw($default_value, $maxlen)
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
always_populate_raw_post_data = -1
--POST_RAW--
hello world
--FILE--
<?php

var_dump(sg::getRaw());
var_dump(sg::getRaw(null, 5));
var_dump(file_get_contents("php://input"));

?>
--EXPECT--
string(11) "hello world"
string(5) "hello"
string(11) "hello world"

