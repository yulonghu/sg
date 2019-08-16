--TEST--
Check for Sg::get so easy
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php

//ini_set('sg.func_name',  'test::check');

//$_GET['user'] = base64_encode('Hello World');

class test {
    public static function check($data) {
        return base64_decode($data);
    }
}

//var_dump(sg::get('g.user', 'def'));
//var_dump(sg::get('g.user', 'def'));

//var_dump(sg::get('found', 'xxxxxyyyyyyyyyyyyy'. time()));

$str0 = "hello";
$str1 = "world";

$data = $str0 . $str1;

sg::get('key', $data);
?>
--EXPECT--
