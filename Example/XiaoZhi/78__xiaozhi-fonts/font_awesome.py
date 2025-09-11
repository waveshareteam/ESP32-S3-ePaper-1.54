import os
import sys
import argparse

# map to font awesome
emoji_mapping = {
    "neutral": 0xf5a4,  # 0xf5a4 [blank]  0xf11a 😐
    "happy": 0xf118,    # 😊
    "laughing": 0xf59b, # 😆
    "funny": 0xf588,    # 😂
    "sad": 0xe384,      # 😔
    "angry": 0xf556,    # 😡
    "crying": 0xf5b3,   # 😭
    "loving": 0xf584,   # 😍
    "embarrassed": 0xf579, # 😳
    "surprised": 0xe36b,   # 😲
    "shocked": 0xe375,     # 😱
    "thinking": 0xe39b,    # 🤔
    "winking": 0xf4da,     # 😉
    "cool": 0xe398,        # 😎
    "relaxed": 0xe392,     # 😌
    "delicious": 0xe372,   # 😋
    "kissy": 0xf598,       # 😗
    "confident": 0xe409,   # 😏
    "sleepy": 0xe38d,      # 😴
    "silly": 0xe3a4,       # 😜
    "confused": 0xe36d     # 😕
}

# map to font awesome icons
icon_mapping = {
    # battery icons
    "battery_full": 0xf240,
    "battery_3": 0xf241,
    "battery_2": 0xf242, 
    "battery_1": 0xf243,
    "battery_empty": 0xf244,
    "battery_slash": 0xf377,
    "battery_charging": 0xf376,

    # wifi icons
    "wifi": 0xf1eb,
    "wifi_fair": 0xf6ab,
    "wifi_weak": 0xf6aa,
    "wifi_off": 0xf6ac,

    # signal icons
    "signal_full": 0xf012,
    "signal_4": 0xf68f,
    "signal_3": 0xf68e,
    "signal_2": 0xf68d,
    "signal_1": 0xf68c,
    "signal_off": 0xf695,

    # volume icons
    "volume_high": 0xf028,
    "volume_medium": 0xf6a8,
    "volume_low": 0xf027,
    "volume_mute": 0xf6a9,

    # media controls
    "music": 0xf001,
    "check": 0xf00c,
    "xmark": 0xf00d,
    "power": 0xf011,
    "gear": 0xf013,
    "trash": 0xf1f8,
    "home": 0xf015,
    "image": 0xf03e,
    "edit": 0xf044,
    "prev": 0xf048,
    "next": 0xf051,
    "play": 0xf04b,
    "pause": 0xf04c,
    "stop": 0xf04d,
    "mic"

    # arrows
    "arrow_left": 0xf060,
    "arrow_right": 0xf061,
    "arrow_up": 0xf062,
    "arrow_down": 0xf063,

    # misc icons
    "warning": 0xf071,
    "bell": 0xf0f3,
    "location": 0xf3c5,
    "globe": 0xf0ac,
    "location_arrow": 0xf124,
    "sd_card": 0xf7c2,
    "bluetooth": 0xf293,
    "comment": 0xf075,
    "ai_chip": 0xe1ec,
    "user": 0xf007,
    "user_robot": 0xe04b,
    "download": 0xf019
}

def parse_arguments():
    parser = argparse.ArgumentParser(description='Font Awesome converter utility')
    parser.add_argument('type', choices=['lvgl', 'dump', 'utf8'], help='Output type: lvgl, dump, or utf8')
    parser.add_argument('--font-size', type=int, default=14, help='Font size (default: 14)')
    parser.add_argument('--bpp', type=int, default=4, help='Bits per pixel (default: 2)')
    return parser.parse_args()

def get_font_file(font_size):
    if font_size == 30:
        return "../../tmp/fa-light-300.ttf"
    return "../../tmp/fa-regular-400.ttf"

def generate_utf8_header(emoji_mapping, icon_mapping):
    print('#ifndef FONT_AWESOME_SYMBOLS_H')
    print('#define FONT_AWESOME_SYMBOLS_H')
    print('')
    
    symbols = {}
    for k, v in emoji_mapping.items():
        symbols["emoji_" + k] = v
    for k, v in icon_mapping.items():
        symbols[k] = v
        
    for k, v in symbols.items():
        ch = chr(v)
        utf8 = ''.join(f'\\x{c:02x}' for c in ch.encode("utf-8"))
        print(f'#define FONT_AWESOME_{k.upper()} "{utf8}"')
    
    print('')
    print('#endif')

def main():
    args = parse_arguments()
    
    symbols = list(emoji_mapping.values()) + list(icon_mapping.values())
    flags = "--no-compress --no-prefilter --force-fast-kern-format"
    font = get_font_file(args.font_size)

    if args.type == "utf8":
        output = f"font_awesome_{args.font_size}_{args.bpp}.h"
        generate_utf8_header(emoji_mapping, icon_mapping)
        return 0
    
    symbols_str = ",".join(map(hex, symbols))
    
    if args.type == "lvgl":
        output = f"src/font_awesome_{args.font_size}_{args.bpp}.c"
        cmd = f"lv_font_conv {flags} --font {font} --format lvgl --lv-include lvgl.h --bpp {args.bpp} -o {output} --size {args.font_size} -r {symbols_str}"
    else:  # dump
        output = f"./build"
        cmd = f"lv_font_conv {flags} --font {font} --format dump --bpp {args.bpp} -o {output} --size {args.font_size} -r {symbols_str}"

    print("Total symbols:", len(symbols))
    print("Generating", output)

    ret = os.system(cmd)
    if ret != 0:
        print(f"命令执行失败，返回码：{ret}")
        return ret
    print("命令执行成功")
    return 0

if __name__ == "__main__":
    sys.exit(main())

