# D2工具箱

仅用于客户端(Diablo II 1.13c)。配置文件是dll所在目录的d2ext.ini

本插件仅供研究学习与离线单机游戏使用。  
**严禁将本项目用于任何商业用途或在线作弊行为。**

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
;; 按下空格 合成 (需打开盒子界面才生效)
transmute = space
;; 按下CTRL和L重新加载配置
reload = CTRL+L
```

## 快捷施法（类似重置版）

``` ini
[helper.fastcast]
;;; 默认开启/关闭
enable = true
;;; 开启/关闭切换热键。格式类似是CTRL+ALT+SHIFT+G
;;;   例如: toggleKey = CTRL        ;;; 按CTRL时切换
;;;        toggleKey = CTRL+ALT+G   ;;; 同时按下CTRL ALT和G时切换
toggleKey = CTRL+G

;; 急速切回原技能。需要打两处补丁
;; quickSwapBack = true

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


;;; 键盘按住时自动重复释放技能。默认开启
;;; repeatOnDown = true
;;; repeatOnDownDelay = 100

```
技能id参见 [skills.md](https://github.com/dabeibao/d2helper/blob/main/examples/skills.md)

## 连续施法
夜妖风群给的正式名字O(∩_∩)O
需要开启快捷施法才能生效

``` ini
[helper.autorepeat]
;; 连续施法
;;; 开启/关闭。默认开启。需设置skills后才生效
;;; enable = true

;;; 开启/关闭连续施法快捷键
;;; toggleKey = CTRL+R

;;; 连续施法技能列表, 默认空
;;; 盾击，热诚启用连续施法
;;; skills = 97,106


;;; 连续施法延迟，单位毫秒。默认100
;;; delay = 100

;;; 如何停止连续施法。默认是7
;;;   1: 按任意其他技能键关闭
;;;   2: 按鼠标左键关闭
;;;   4: 按鼠标右键关闭
;;;   8: 再次按该技能键关闭
;;; 可组合。例如，选择9 表示按其他技能键和该技能键都会停止; 6表示按任意鼠标键停止
;;; stopMode = 7
```


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
