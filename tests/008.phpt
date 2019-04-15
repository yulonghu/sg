--TEST--
Check for Sg Management $_SESSION
--SKIPIF--
<?php if (!extension_loaded("sg")) print "skip"; ?>
--INI--
sg.enable = 1
--FILE--
<?php
ob_start();

echo "------------------\$_SESSION init\n";
session_start();
$_SESSION['a'] = 'session_val_a';
$_SESSION['b'] = 'session_val_b';
$_SESSION['t'][0] = 'session_val_t_0';
$_SESSION['t'][1] = 'session_val_t_1';

var_dump($_SESSION);
var_dump(sg::get('n'));

echo "------------------\$_SESSION set\n";
$_SESSION['a'] = 'session_val_a is ori change';
var_dump($_SESSION);
var_dump(sg::get('n'));

echo "------------------sg::set\n";
var_dump(sg::set('n.a', 'session_val_a is sg change'));
var_dump($_SESSION);
var_dump(sg::get('n'));

echo "------------------sg::del\n";
var_dump(sg::del(sg::del('n.a')));
var_dump($_SESSION);
var_dump(sg::get('n'));

echo "------------------\$_SESSION unset\n";
unset($_SESSION['t'][0]);
var_dump($_SESSION);
var_dump(sg::get('n'));
?>
--EXPECT--
------------------$_SESSION init
array(3) {
  ["a"]=>
  string(13) "session_val_a"
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(2) {
    [0]=>
    string(15) "session_val_t_0"
    [1]=>
    string(15) "session_val_t_1"
  }
}
array(3) {
  ["a"]=>
  string(13) "session_val_a"
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(2) {
    [0]=>
    string(15) "session_val_t_0"
    [1]=>
    string(15) "session_val_t_1"
  }
}
------------------$_SESSION set
array(3) {
  ["a"]=>
  string(27) "session_val_a is ori change"
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(2) {
    [0]=>
    string(15) "session_val_t_0"
    [1]=>
    string(15) "session_val_t_1"
  }
}
array(3) {
  ["a"]=>
  string(27) "session_val_a is ori change"
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(2) {
    [0]=>
    string(15) "session_val_t_0"
    [1]=>
    string(15) "session_val_t_1"
  }
}
------------------sg::set
bool(true)
array(3) {
  ["a"]=>
  string(26) "session_val_a is sg change"
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(2) {
    [0]=>
    string(15) "session_val_t_0"
    [1]=>
    string(15) "session_val_t_1"
  }
}
array(3) {
  ["a"]=>
  string(26) "session_val_a is sg change"
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(2) {
    [0]=>
    string(15) "session_val_t_0"
    [1]=>
    string(15) "session_val_t_1"
  }
}
------------------sg::del
bool(false)
array(2) {
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(2) {
    [0]=>
    string(15) "session_val_t_0"
    [1]=>
    string(15) "session_val_t_1"
  }
}
array(2) {
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(2) {
    [0]=>
    string(15) "session_val_t_0"
    [1]=>
    string(15) "session_val_t_1"
  }
}
------------------$_SESSION unset
array(2) {
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(1) {
    [1]=>
    string(15) "session_val_t_1"
  }
}
array(2) {
  ["b"]=>
  string(13) "session_val_b"
  ["t"]=>
  array(1) {
    [1]=>
    string(15) "session_val_t_1"
  }
}

