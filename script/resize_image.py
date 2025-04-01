from PIL import Image

def resize_image_with_padding(input_path, output_path, target_width, min_height):
    img = Image.open(input_path)
    original_width, original_height = img.size
    
    # 计算等比例缩放后的高度
    ratio = target_width / original_width
    new_height = int(original_height * ratio)
    
    # 缩放图像
    resized_img = img.resize((target_width, new_height), Image.LANCZOS)
    
    # 如果高度不足则填充黑色
    if new_height < min_height:
        new_img = Image.new('RGB', (target_width, min_height), (0, 0, 0))
        y = (min_height - new_height) // 2
        new_img.paste(resized_img, (0, y))
        # 明确指定保存为PNG格式
        new_img.save(output_path, format="PNG")
    else:
        # 直接保存缩放后的图像为PNG
        resized_img.save(output_path, format="PNG")

# 示例用法（输出路径改为.png）
resize_image_with_padding("image_1_400_300.jpg", "image_1_400_300.png", 142, 428)
resize_image_with_padding("image_2_400_300.jpg", "image_2_400_300.png", 142, 428)
resize_image_with_padding("image_3_400_300.jpg", "image_3_400_300.png", 142, 428)