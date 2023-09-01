# D2工具箱

仅用于客户端。配置文件是dll所在目录的d2ext.ini

## 全局配置

``` ini
[helper.config]
;;; 打开调试信息
debug = 1

;;; 更多打印信息
verbose = 1
```

## 快捷键合成

``` ini
[helper.key]
;;; 合成快捷键. 快捷键是VK_的十进制值
transmute = DDD
```

## 快速施法（类似重置版）

``` ini
[helper.fastcast]
;;; 默认开启/关闭
enable = true
;;; 开启/关闭切换热键。格式类似是CTRL+ALT+SHIFT+G
;;;   例如: toggleKey = CTRL        ;;; 按CTRL时切换
;;;        toggleKey = CTRL+ALT+G   ;;; 同时按下CTRL ALT和G时切换
toggleKey = CTRL+G


;; attackSkills = xxx
;;     左手使用这些技能时将不按shift键
;;     前面加-号表示将技能移除出列表
;;     例如:  attackSkills = -0  普通攻击将会原地攻击
;;;attackSkills = 
```
技能id参见 [skills.md](https://github.com/dabeibao/d2helper/blob/main/examples/skills.md)

## 注册表沙盒

``` ini
[helper.regsim]
;;; 注册表沙盒。注册表读写将定向到dll目录的D2RegSimPre.ini和D2RegSim.ini。
;;;   其中D2RegSimPre.ini为预设值的配置，不会被修改。可用于保存战网IP之类的

;;; 开启/关闭。默认开启
enable = true
```
注册表格式参见
   [D2RegSim.ini](https://github.com/dabeibao/d2helper/blob/main/examples/D2RegSim.ini)

## 加载d2hack.script

``` ini
[helper.hackscript]
;;; 开启/关闭。默认关闭
enable = false

;;; 加载的脚本名字。默认d2hack.script
;; file = d2hack.script
```
