# SG - PHP Superglobals Management
[![Build Status](https://travis-ci.org/yulonghu/sg.svg?branch=master)](https://travis-ci.org/yulonghu/sg)

一个超级简单的PHP超全局变量管理扩展

[English Document](https://github.com/yulonghu/sg/blob/master/README.md)

### 介绍

[SG](https://github.com/yulonghu/sg) 全称 [Superglobals](http://php.net/manual/en/language.variables.superglobals.php)，它的诞生为了方便快捷操作PHP预定义的超全局变量，用户定义的超全局变量。

如果在非CLI模式，SG默认托管PHP预定义的超全局变量, 包括常用 $_GET，$_POST，$_COOOKIE，$_SERVER，$_FILES。

使用SG类，可以大大的减少我们的PHP代码量，提高我们的开发效率。

### 亮点功能
- 简单，快速，轻量
- 零拷贝访问全局变量
- 支持自动过滤前后空格[PHP trim](http://php.net/manual/en/function.trim.php)
- 解决操作PHP全局变量时出现未定义系列的问题 (Undefined variable, Undefined offset)

## 安装
### PHP版本支持包括
- PHP 5.4 +
- PHP 7.0 +

### 下载源码
```
git clone https://github.com/yulonghu/sg.git
```

### Linux系统，编译SG扩展
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

重启php进程，就安装成功啦。

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

|传统的获取方式 (短)|新获取方式|
| ------ | ------ |
|$_GET['key']|sg::get('g.key')|
|$_POST['key']|sg::get('p.key')|
|$_COOKIE['key']|sg::get('c.key')|
|$_SERVER['key']|sg::get('s.key')|
|$_FILES['key']|sg::get('f.key')|

|传统的获取方式 (长)|新获取方式|
| ------ | ------ |
|$_GET['key']['key1']['key2']|sg::get('g.key.key1.key2')|
|$_POST['key']['key1']['key2']|sg::get('p.key.key1.key2')|
|$_COOKIE['key']['key1']['key2']|sg::get('c.key.key1.key2')|
|$_SERVER['key']['key1']['key2']|sg::get('s.key.key1.key2')|
|$_FILES['key']['key1']['key2']|sg::get('f.key.key1.key2')|

|传统的获取方式 (isset + trim)|新获取方式|
| ------ | ------ |
|$key = isset($_GET['key']) ? trim($_GET['key']) : null;|$key = sg::get('g.key');|
|$key = isset($_POST['key']) ? trim($_POST['key']) : null;|$key = sg::get('p.key');|
|$key = isset($_COOKIE['key']) ? trim($_COOKIE['key']) : null;|$key = sg::get('c.key');|
|$key = isset($_SERVER['key']) ? trim($_SERVER['key']) : null;|$key = sg::get('s.key');|
|$key = isset($_FILES['key']) ? trim($_FILES['key']) : null;|$key = sg::get('f.key');|

|(PHP7) 传统的获取方式 (??)|新获取方式|
| ------ | ------ |
|$key = $_GET['key']) ?? null; $key = trim($key);|$key = sg::get('g.key');|
|$key = $_POST['key'] ?? null; $key = trim($key);|$key = sg::get('p.key');|
|$key = $_COOKIE['key'] ?? null; $key = trim($key);|$key = sg::get('c.key');|
|$key = $_SERVER['key'] ?? null; $key = trim($key);|$key = sg::get('s.key');|
|$key = $_FILES['key'] ?? null; $key = trim($key);|$key = sg::get('f.key');|

通过以上的整理，可以得出一个结论，传统取值方式容易出错, 如果数组维度越深，代码量会直线上升。

使用SG，这些情况都变得很简单。更新、删除方式类同。

### 设置超全局变量

#### bool sg::set(string $key, mixed $value)
```php
<?php
var_dump(sg::set('test', 'test apple'));
var_dump(sg::set('user.0.0', 'user 0 apple'));
var_dump(sg::set('user.0.1', 'user 1 apple'));
var_dump(sg::set('user.a.a', 'user a apple'));
var_dump(sg::set('user.a.b', 'user b apple'));

// 以下方式不推荐，更新PHP预定义的超全局变量值
var_dump(sg::set('g.key', 'value'));
var_dump(sg::set('p.key', 'value'));
var_dump(sg::set('c.key', 'value'));
var_dump(sg::set('s.key', 'value'));
var_dump(sg::set('f.key', 'value'));
```
以上例子输出的结果:
```txt
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)

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

#### bool sg::has(string $key)
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

#### bool sg::del(string $key)
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

演示自动过滤前后空格的例子, 支持两种方式开启自动过滤。

为了获得最佳性能，如果满足trim条件, sg将改变原始值。避免每次取值时重复做trim操作。

```php
<?php
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

