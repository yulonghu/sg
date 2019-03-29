# SG - PHP Superglobals Management
[![Build Status](https://travis-ci.org/yulonghu/sg.svg?branch=master)](https://travis-ci.org/yulonghu/sg)

一个超级简单的PHP超全局变量管理扩展

[English Document](https://github.com/yulonghu/sg/blob/master/README.md)

### 介绍

SG 全称 [Superglobals](http://php.net/manual/en/language.variables.superglobals.php)，它的诞生为了方便快捷操作PHP预定义的超全局变量，用户定义的超全局变量。

如果在非CLI模式，SG默认托管PHP预定义的超全局变量, 包括$_GET, $_POST, $_COOOKIE, $_SERVER, $_FILES。

使用SG类，可以大大的减少我们的PHP代码量，提高我们的开发效率。

### 亮点功能
- 简单，快速，轻量
- 零拷贝访问全局变量
- 支持自动过滤前后空格[PHP trim](http://php.net/manual/en/function.trim.php)
- 解决操作PHP全局变量时出现未定义系列的问题 (Undefined variable, Undefined offset)

## 安装
### PHP版本最低要求
- PHP 5.4 +

### 下载源码
```
git clone https://github.com/yulonghu/sg.git
```

### 在Linux系统中编译PHP SG扩展
```
$ /path/to/php/bin/phpize
$ ./configure --with-php-config=/path/to/php/bin/php-config
$ make && make install
```

#### php.ini，添加、开启扩展
```ini
extension=sg.so

[sg]
sg.enable = On
```

重启php-fpm，就安装成功啦。

## 提供的方法
```php
mixed sg::get(string $key [, mixed $default_value = null])
bool sg::set(string $key, mixed $value)
bool sg::has(string $key)
bool sg::del(string $key)
```

## 支持的INI配置项
```ini
sg.enable = On/Off
sg.auto_trim = On/Off ; Strip whitespace with PHP trim
```
## 详细例子

### 获取PHP预定义的超全局变量

|OLD GET METHOD (Short)|NEW GET METHOD|
| ------ | ------ |
|$_GET['key']|sg::get('g.key')|
|$_POST['key']|sg::get('p.key')|
|$_COOKIE['key']|sg::get('c.key')|
|$_SERVER['key']|sg::get('s.key')|
|$_FILES['key']|sg::get('f.key')|

|OLD GET METHOD (Long)|NEW GET METHOD|
| ------ | ------ |
|$_GET['key']['key1']['key2']|sg::get('g.key.key1.key2')|
|$_POST['key']['key1']['key2']|sg::get('p.key.key1.key2')|
|$_COOKIE['key']['key1']['key2']|sg::get('c.key.key1.key2')|
|$_SERVER['key']['key1']['key2']|sg::get('s.key.key1.key2')|
|$_FILES['key']['key1']['key2']|sg::get('f.key.key1.key2')|

### 设置超全局变量

#### sg::set
```php
<?php
var_dump(sg::set('test', 'test apple'));
var_dump(sg::set('user.0.0', 'user 0 apple'));
var_dump(sg::set('user.0.1', 'user 1 apple'));
var_dump(sg::set('user.a.a', 'user a apple'));
var_dump(sg::set('user.a.b', 'user b apple'));
```
以上例子输出的结果:
```txt
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
```

#### sg::get
```php
<?php
var_dump(sg::get('test', 'test apple'));
var_dump(sg::get('user');
var_dump(sg::get('not_found', 'def');
var_dump(sg::get('user.1.2.3.4'));
```
以上例子输出的结果:
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
#### sg::has
```php
<?php
var_dump(sg::has('test'));
var_dump(sg::has('not_found'));
```
以上例子输出的结果:
```
bool(true)
bool(false)
```
#### sg::del
```php
<?php
var_dump(sg::del('test'));
var_dump(sg::del('user.0.1'));
var_dump(sg::get('user');
```
以上例子输出的结果:
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

### sg.auto_trim

演示自动过滤前后空格的例子, 两种方式开启自动过滤。

```php
<?php
// sg.auto_trim = On ; php.ini
ini_set('sg.auto_trim', 1);
function One() {
    var_dump(sg::set('test', ' test apple ')); // Auto-call PHP trim
}
function Two() {
    var_dump(sg::get('test'));
}
```
以上例子输出的结果:
```txt
bool(true)
string(10) "test apple"
```

## License
SG is open source software under the [PHP License v3.01](http://www.php.net/license/3_01.txt)

