# SG - PHP Syntax Sugar Extension

[![Build Status](https://travis-ci.org/yulonghu/sg.svg?branch=master)](https://travis-ci.org/yulonghu/sg)

A simple PHP syntax sugar extension

[中文文档](https://github.com/yulonghu/sg/blob/master/README_ZH.md)

### Introduction

[SG](https://github.com/yulonghu/sg) Full name [Superglobals](http://php.net/manual/en/language.variables.superglobals.php), References all variables available in global scope, SG has extended a new way to manage PHP superglobals variables, Make the management of PHP superglobals variables simple and unified.

These superglobal variables managed by SG are: $_SERVER, $_GET, $_POST, $_FILES, $_COOKIE, $_SESSION, $_REQUEST, $_ENV.

Very important point: it is very simple.

### Features
- Simple, Fast, Lightweight
- Access PHP superglobals zero-copy, Synchronously update PHP Superglobals variables
- Support for custom function, Default call [PHP trim](http://php.net/manual/en/function.trim.php)
- Solve the problem of undefined series when using PHP Superglobals variables (Undefined variable, Undefined offset)
- Use static function method, Replace the PHP array dimension with a decimal point
- Use global statement, Replace the PHP array dimension with a underline
- Support for global key the option configuration, Default level one lookup

## Install
### Supported Version
- PHP 5.4 +
- PHP 7.0 +

### DownLoad
```
git clone https://github.com/yulonghu/sg.git
```

### Compile SG in Linux
```
$ /path/to/php/bin/phpize
$ ./configure --with-php-config=/path/to/php/bin/php-config
$ make && make install
```

#### Add the follow information to your php.ini
```ini
extension=sg.so

[sg]
sg.enable = On
```

Restart the php-fpm.

## Help Manual

### Methods
```php
array sg::all(void)
mixed sg::get(string $key [, mixed $default_value = null])
bool sg::set(string $key, mixed $value)
bool sg::has(string $key)
bool sg::del(string $key [, mixed $... ])
```

### Inis(php.ini)
| Options | Permission | Type | Default | Desc |
|---|---|---|---|---|
|sg.enable|PHP_INI_SYSTEM|bool|0| 0 Trun-Off 1 Turn-On|
|sg.global_level|PHP_INI_SYSTEM|bool|1| 1 Limit Level 0 Unlimited Level|
|sg.func_name|PHP_INI_ALL|char|trim| [PHP trim](http://php.net/manual/en/function.trim.php), Support for custom function|

### Hash Map
- When management PHP sessions (MapKey = n), First call the function session_start()

|PHP Predefined Superglobals|SG Key|global statement| Method Example|
| ------ | ------ | ------ | ------ |
|$GLOBALS|-|-|sg::all()|
|$_SERVER|s|global $s|sg::get/set/has/del('s')|
|$_GET|g|global $g|sg::get/set/has/del('g')|
|$_POST|p|global $p|sg::get/set/has/del('p')|
|$_FILES|f|global $f|sg::get/set/has/del('f')|
|$_COOKIE|c|global $c|sg::get/set/has/del('c')|
|$_SESSION|n|global $n|sg::get/set/has/del('n')|
|$_REQUEST|r|global $r|sg::get/set/has/del('r')|
|$_ENV|e|global $e|sg::get/set/has/del('e')|

### Usage

#### bool sg::set(string $key, mixed $value)
```php
<?php
var_dump(sg::set('test', 'test apple')); // Same as $GLOBALS['test'] = 'test apple'
var_dump(sg::set('user.0.0', 'user 0 apple')); // Same as $GLOBALS['test'][0][0]
var_dump(sg::set('user.0.1', 'user 1 apple')); // Same as $GLOBALS['test'][0][1]
var_dump(sg::set('user.a.a', 'user a apple')); // Same as $GLOBALS['test']['a']'a']
var_dump(sg::set('user.a.b', 'user b apple')); // Same as $GLOBALS['test']['a']'b']
```

The above example will output:
```txt
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
```

#### mixed sg::get(string $key [, mixed $default_value = null])
```php
<?php
var_dump(sg::get('test', 'test apple')); 
var_dump(sg::get('user');
var_dump(sg::get('not_found', 'def');
var_dump(sg::get('user.1.2.3.4'));
```
The above example will output:
```txt
string(10) "test apple"
array(2) {
  [0]=>
  array(2) {
    [0]=>
    string(12) "user 0 apple"
    [1]=>
    string(12) "user 1 apple"
  }
  ["a"]=>
  array(2) {
    ["a"]=>
    string(12) "user a apple"
    ["b"]=>
    string(12) "user b apple"
  }
}
string(3) "def"
NULL
```

#### bool sg::has(string $key)
```php
<?php
var_dump(sg::has('test'));
var_dump(sg::has('not_found'));
```
The above example will output:
```
bool(true)
bool(false)
```

#### bool sg::del(string $key [, mixed $... ])
```php
<?php
var_dump(sg::del('test'));
var_dump(sg::del('user.0.1'));
var_dump(sg::get('user');
```
The above example will output:
```
bool(true)
bool(true)
array(2) {
  [0]=>
  array(1) {
    [0]=>
    string(12) "user 0 apple"
  }
  ["a"]=>
  array(2) {
    ["a"]=>
    string(12) "user a apple"
    ["b"]=>
    string(12) "user b apple"
  }
}
```

### array sg::all(void)
The same result as [$GLOBALS](https://www.php.net/manual/zh/reserved.variables.globals.php)

## License
SG is open source software under the [PHP License v3.01](http://www.php.net/license/3_01.txt)

