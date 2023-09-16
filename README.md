# D2工具箱

仅用于客户端(Diablo II 1.13c)。配置文件是dll所在目录的d2ext.ini

## 全局配置

``` ini
[helper.config]
;;; 打开调试信息
debug = 0

;;; 更多打印信息
verbose = 0
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
;;    左手释放时, 模拟近战攻击, 鼠标点空地人物会移动过去
;;    例如:  attackSkills = 0
;;;attackSkills =

;; castSkills = xxx
;;    左手释放时，模拟法术攻击，鼠标点空地人物保持不动（需要配置站立不动键)
;;    例如: castSkils = 12
;;;castSkills =


;; 灵气技能
;;; 不复位灵气技能。默认开启
;;; keepAuraSkills = true
;;; 灵气技能列表。默认已经加入了所有灵气技能，无需设置。前面加-表示移除出默认列表
;;; auraSkills = 99,100


;; 自动重复释放技能
;;; 开启自动重复释放技能。默认关闭
;;; autoRepeatEnabled = false

;;; 自动重复释放技能列表
;;; 盾击，热诚自动重复释放
;;; autoRepeatSkills = 97,106

;;; 开启关闭自动重复快捷键。
;;; autoRepeatToggleKey = CTRL+R

;;; 自动重复延迟，单位毫秒。默认100
;;; autoRepeatDelay = 100

;;; 停止重复释放技能，默认需要再按一次技能键
;;;   1: 按任意技能键关闭
;;;   2: 按鼠标左键关闭
;;;   4: 按鼠标右键关闭
;;;   8: 再次按该技能键关闭
;;; 可组合。默认是7
;;;autoRepeatStop = 7

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
