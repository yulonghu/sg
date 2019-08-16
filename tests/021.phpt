--TEST--
Check for sg::getRaw(void) auto call Example::decrypt
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
always_populate_raw_post_data = -1
--POST_RAW--
aHR0cCUzQSUyRiUyRnd3dy5ib3gzLmNu
--FILE--
<?php

ini_set('sg.func_name',  'Example::decrypt');

class Example {
    public static function decrypt($data) {
        return urldecode(base64_decode($data));
    }
}

var_dump(sg::getRaw());
var_dump(file_get_contents("php://input"));
var_dump(sg::getRaw());

?>
--EXPECT--
string(18) "http://www.box3.cn"
string(32) "aHR0cCUzQSUyRiUyRnd3dy5ib3gzLmNu"
string(18) "http://www.box3.cn"

