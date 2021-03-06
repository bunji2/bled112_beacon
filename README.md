# bled112 を用いたビーコン実装例

## beacon.exe の使い方

### コマンドラインヘルプの表示

```
C:\work>beacon
beacon <list|COMx <info|stop|start config_filename>>
```
### bled112 のシリアルポートの表示

```
C:\work>beacon list
Bluegiga Bluetooth Low Energy (COM4)
```

### ビーコンのアドバタイジング開始

引数の config.txt は設定ファイルのパス。

```
C:\work>beacon COM4 start config.txt
uuid=ffffffffffffffffffffffffffffffff
major=0x00ff
minor=0x00fe
txpower=-58
adv_interval_min=160
adv_interval_max=160
adv_channels=0x07
```

### ビーコンのアドバタイジング終了

```
C:\work>beacon COM4 stop
```

### 設定ファイルの記述例

```
# Configuration for bled112_beacon
# [Format]
#   Every line must be in 80bytes.
#   The line that starts with "#" is comment.

# uuid (Hexadecimal, 32byte)
#   beacon uuid
#   Example: uuid=e2c56db5dffb48d2b060d0f5a71096e0
uuid=FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

# major
#   Major number
#   Range: 0-255
major=0xFF

# minor
#   Minor number
#   Range: 0-255
minor=254

# txpower
#   Tx power
#   Range: -127 - 127
txpower=-58

# adv_interval_min
#   Minimum advertisement interval in units of 625us
#   Range: 0x20 to 0x4000
#   Default: 0x200 (320ms)
#   Example: 0x200 = 512. 512 * 625us = 320000us = 320ms
adv_interval_min=160

# adv_interval_max
#   Maximum advertisement interval in units of 625us
#   Range: 0x20 to 0x4000
#   Default: 0x200 (320ms)
#   Example: 0x200 = 512. 512 * 625us = 320000us = 320ms
adv_interval_max=0xA0

# adv_channels
#   A bit mask to identify which of the three advertisement channels are used
#   Excamle:
#     7: all three channels are used,
#     3: advertisement channels 37 and 38 are used,
#     4: only advertisement channel 39 is used
adv_channels=0x07
```

* 先頭の一文字目が "#" の行はコメント行。
* 名前=値のは設定を行う行だが、余分な空白やタブは許容されない。
