#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
文本转图片程序
使用 AlibabaPuHuiTi-3-35-Thin 字体生成文本图片
"""

from PIL import Image, ImageDraw, ImageFont
import os
import argparse

def create_text_image(text, font_path, output_path="output.png", 
                     font_size=48, bg_color=(255, 255, 255), 
                     text_color=(0, 0, 0), padding=20):
    """
    根据文本创建图片
    
    Args:
        text (str): 要渲染的文本
        font_path (str): 字体文件路径
        output_path (str): 输出图片路径
        font_size (int): 字体大小
        bg_color (tuple): 背景颜色 (R, G, B)
        text_color (tuple): 文本颜色 (R, G, B)
        padding (int): 内边距
    """
    
    # 检查字体文件是否存在
    if not os.path.exists(font_path):
        raise FileNotFoundError(f"字体文件不存在: {font_path}")
    
    try:
        # 加载字体
        font = ImageFont.truetype(font_path, font_size)
    except Exception as e:
        raise Exception(f"无法加载字体文件: {e}")
    
    # 创建临时图片来测量文本尺寸
    temp_img = Image.new('RGB', (1, 1), bg_color)
    temp_draw = ImageDraw.Draw(temp_img)
    
    # 获取文本边界框
    bbox = temp_draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    
    # 计算图片尺寸
    img_width = text_width + 2 * padding
    img_height = text_height + 2 * padding
    
    # 创建图片
    image = Image.new('RGB', (img_width, img_height), bg_color)
    draw = ImageDraw.Draw(image)
    
    # 绘制文本
    draw.text((padding, padding), text, font=font, fill=text_color)
    
    # 保存图片
    image.save(output_path)
    print(f"图片已保存到: {output_path}")
    print(f"图片尺寸: {img_width} x {img_height}")
    
    return image

def main():
    parser = argparse.ArgumentParser(description='将文本转换为图片')
    parser.add_argument('text', help='要渲染的文本')
    parser.add_argument('-f', '--font', 
                       default='AlibabaPuHuiTi-3-35-Thin/AlibabaPuHuiTi-3-35-Thin.ttf',
                       help='字体文件路径')
    parser.add_argument('-o', '--output', default='output.png',
                       help='输出图片路径')
    parser.add_argument('-s', '--size', type=int, default=48,
                       help='字体大小')
    parser.add_argument('--bg-color', default='255,255,255',
                       help='背景颜色 (R,G,B)')
    parser.add_argument('--text-color', default='0,0,0',
                       help='文本颜色 (R,G,B)')
    parser.add_argument('--padding', type=int, default=20,
                       help='内边距')
    
    args = parser.parse_args()
    
    # 解析颜色参数
    bg_color = tuple(map(int, args.bg_color.split(',')))
    text_color = tuple(map(int, args.text_color.split(',')))
    
    try:
        create_text_image(
            text=args.text,
            font_path=args.font,
            output_path=args.output,
            font_size=args.size,
            bg_color=bg_color,
            text_color=text_color,
            padding=args.padding
        )
    except Exception as e:
        print(f"错误: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    # 示例用法
    if len(os.sys.argv) == 1:
        # 如果没有命令行参数，使用示例文本
        example_text = "让天下没有难做的生意こんにちは안녕하세요 ПриветمرحباOláनमस्तेสวัสดีTạm biệt"
        print("使用示例文本生成图片...")
        try:
            create_text_image(
                text=example_text,
                font_path="build/puhui-7415-Regular.ttf",
                output_path="example_output.png",
                font_size=36,
                bg_color=(255, 255, 255),
                text_color=(0, 0, 0),
                padding=30
            )
        except Exception as e:
            print(f"错误: {e}")
    else:
        exit(main()) 