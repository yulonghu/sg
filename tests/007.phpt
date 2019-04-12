--TEST--
Check for sg::all()
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php

$_GET = array();
$_POST = array();
$_COOKIE = array();
$_SERVER = array();
$_FILES = array();

$_GET['gt'] = 'g_hello';
$_GET['ga'][0][0] = 'get_a';
$_GET['ga'][0][1] = 'get_b';

$_POST['pt'] = 'p_hello';
$_POST['ga'][0][0] = 'post_a';
$_POST['ga'][0][1] = 'post_b';

$_COOKIE['ct'] = 'c_hello';
$_COOKIE['ca'][0][0] = 'cookie_a';
$_COOKIE['ca'][0][1] = 'cookie_b';

$_SERVER['st'] = 's_hello';
$_SERVER['sa'][0][0] = 'server_a';
$_SERVER['sa'][0][1] = 'server_b';

$_FILES['ft'] = 'f_hello';
$_FILES['fa'][0][0] = 'files_a';
$_FILES['fa'][0][1] = 'files_b';

sg::set('sgt', 'sgt_hello');
sg::set('sga.0.0', 'sg_a');
sg::set('sga.0.1', 'sg_b');

var_dump(sg::get('g'));
var_dump(sg::get('p'));
var_dump(sg::get('c'));
var_dump(sg::get('s'));
var_dump(sg::get('f'));

var_dump(sg::get('sgt'));
var_dump(sg::get('sga'));

unset($_GET);
unset($_POST);
unset($_COOKIE);
unset($_SERVER);
unset($_FILES);

if ($GLOBALS == sg::all()) {
    var_dump('$GLOBALS is equal to sg::all()');
} else {
    var_dump('$GLOBALS is not equal to sg::all()');
}

?>
--EXPECT--
array(2) {
  ["gt"]=>
  string(7) "g_hello"
  ["ga"]=>
  array(1) {
    [0]=>
    array(2) {
      [0]=>
      string(5) "get_a"
      [1]=>
      string(5) "get_b"
    }
  }
}
array(2) {
  ["pt"]=>
  string(7) "p_hello"
  ["ga"]=>
  array(1) {
    [0]=>
    array(2) {
      [0]=>
      string(6) "post_a"
      [1]=>
      string(6) "post_b"
    }
  }
}
array(2) {
  ["ct"]=>
  string(7) "c_hello"
  ["ca"]=>
  array(1) {
    [0]=>
    array(2) {
      [0]=>
      string(8) "cookie_a"
      [1]=>
      string(8) "cookie_b"
    }
  }
}
array(2) {
  ["st"]=>
  string(7) "s_hello"
  ["sa"]=>
  array(1) {
    [0]=>
    array(2) {
      [0]=>
      string(8) "server_a"
      [1]=>
      string(8) "server_b"
    }
  }
}
array(2) {
  ["ft"]=>
  string(7) "f_hello"
  ["fa"]=>
  array(1) {
    [0]=>
    array(2) {
      [0]=>
      string(7) "files_a"
      [1]=>
      string(7) "files_b"
    }
  }
}
string(9) "sgt_hello"
array(1) {
  [0]=>
  array(2) {
    [0]=>
    string(4) "sg_a"
    [1]=>
    string(4) "sg_b"
  }
}
string(30) "$GLOBALS is equal to sg::all()"

