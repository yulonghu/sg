# SG - PHP Syntax Sugar Extension

[![Build Status](https://travis-ci.org/yulonghu/sg.svg?branch=master)](https://travis-ci.org/yulonghu/sg)

一个简单的PHP语法糖扩展

[English Document](https://github.com/yulonghu/sg/blob/master/README.md)

### 介绍

[SG](https://github.com/yulonghu/sg) 全称 [Superglobals](http://php.net/manual/en/language.variables.superglobals.php)，引用全局作用域中可用的全部变量。SG扩展了一种全新的PHP超全局变量管理方式。

SG管理的这些超全局变量有：$_SERVER，$_GET，$_POST，$_FILES，$_COOKIE，$_SESSION，$_REQUEST，$_ENV。

当然了，也可以应用到自定义变量场景。

非常重要的一点：它很简单！

### 亮点功能
- 简单，快速，轻量
- 零拷贝访问PHP超全局变量，使用SG会同步更新PHP超全局变量
- 支持取值前调用自定义函数，默认情况下，字符串变量会自动调用 [PHP trim](http://php.net/manual/en/function.trim.php)
- 解决使用PHP超全局变量时出现未定义系列的问题 (Undefined variable, Undefined offset)
- 采用函数调用方式时，以小数点代替PHP数组维度
- 采用global声明方式时，以下划线代替PHP数组维度
- 支持global key维度深度可配置，默认一级查找

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

## 帮助手册

### 支持的方法
```php
array sg::all(void)
mixed sg::get(string $key [, mixed $default_value = null])
bool sg::set(string $key, mixed $value)
bool sg::has(string $key)
bool sg::del(string $key [, mixed $... ])
```
### 配置项(php.ini)
| 配置项 | 权限 | 类型 |默认值 | 说明 |
|---|---|---|---|---|
|sg.enable|PHP_INI_SYSTEM|bool|0| 0 关闭 1 开启|
|sg.global_level|PHP_INI_SYSTEM|bool|1| 1 只支持一级查找 0 无限制查找|
|sg.func_name|PHP_INI_ALL|char|trim|默认调用 [PHP trim](http://php.net/manual/en/function.trim.php) 函数，也支持自定义函数 |

### Hash Map
- 管理PHP会话时(MapKey = n), 请先调用函数 session_start()

|PHP 超全局变量|SG key (关键字缩写)|global 声明|函数|
| ------ | ------ | ------ | ------ |
|$GLOBALS|无|无|sg::all()|
|$_SERVER|s|global $s|sg::get/set/has/del('s')|
|$_GET|g|global $g|sg::get/set/has/del('g')|
|$_POST|p|global $p|sg::get/set/has/del('p')|
|$_FILES|f|global $f|sg::get/set/has/del('f')|
|$_COOKIE|c|global $c|sg::get/set/has/del('c')|
|$_SESSION|n|global $n|sg::get/set/has/del('n')|
|$_REQUEST|r|global $r|sg::get/set/has/del('r')|
|$_ENV|e|global $e|sg::get/set/has/del('e')|

### 方法的使用

#### bool sg::set(string $key, mixed $value)

设置全局作用域变量，找不到$key则新增，否则更新其值。

返回值: TRUE 成功，FALSE 失败

```php
<?php
var_dump(sg::set('test', 'test apple')); // 等同于 $GLOBALS['test'] = 'test apple'
var_dump(sg::set('user.0.0', 'user 0 apple')); // 等同于 $GLOBALS['test'][0][0]
var_dump(sg::set('user.0.1', 'user 1 apple')); // 等同于 $GLOBALS['test'][0][1]
var_dump(sg::set('user.a.a', 'user a apple')); // 等同于 $GLOBALS['test']['a']['a']
var_dump(sg::set('user.a.b', 'user b apple')); // 等同于 $GLOBALS['test']['a']['b']
```
以上例子输出的结果:
```txt
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
```
#### mixed sg::get(string $key [, mixed $default_value = null])

读取全局作用域变量, 找不到$key时，$default_value值生效。

```php
<?php
var_dump(sg::get('test', 'test apple')); // 读取一个全局变量test
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

检查全局作用域中$key是否存在

返回值: TRUE 存在，FALSE 不存在

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

#### bool sg::del(string $key [, mixed $... ])

删除全局作用域中变量$key

注意: 批量删除时，有且只有一个成功结果

返回值: TRUE 成功，FALSE 失败

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

### array sg::all(void)
结果与 [$GLOBALS](https://www.php.net/manual/zh/reserved.variables.globals.php) 相同

## License
SG is open source software under the [PHP License v3.01](http://www.php.net/license/3_01.txt)

