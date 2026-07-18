---
title: 软件系统安全初赛华北赛区南开大学1队解题报告
description: CTF竞赛解题报告，包含Re、Web、Crypto等多个方向的题目解析
createdAt: 2026-03-24
updatedAt: 2026-03-24
categories:
  - CTF
  - 解题报告
author: Siamese
image: "@assets/uploads/note1/note1.jpg"
tags:
  - CTF
  - 逆向
  - Web
  - 密码学
draft: false
hideToc: false
---


# 南开大学1队解题报告

> 本文记录南开大学1队在CTF竞赛中的解题过程，包含Re、Web、Crypto等多个方向的题目解析。由HYPERTYPHOON师傅们编写
![逆向分析截图](@assets/uploads/note1/Team.jpg)
## Re1

题目为读取视频的elf文件，运行后得到仅4B的pyc文件，用IDA分析发现为base64编码，转换为pyc文件后再转为py文件，逆向脚本如下：

```python
import cv2
import numpy as np
from tqdm import tqdm
import os

def extract_from_video(video_path="video.mp4",
                       output_file="extracted_payload.bin",
                       width=640,
                       height=480,
                       pixel_size=8):
    
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print("❌ 无法打开视频文件，请确认 video.mp4 存在于当前目录")
        return

    # 基础信息
    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    print(f"视频信息 → 分辨率: {width}x{height} | 帧率: {fps:.1f} fps | 总帧数: {total_frames}")

    bits = ""
    frame_idx = 0

    with tqdm(total=total_frames, desc="正在逐帧提取比特", unit="帧") as pbar:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            frame_idx += 1
            # 确保当前帧大小正确（防止视频被压缩或损坏）
            if frame.shape[1] != width or frame.shape[0] != height:
                print(f"⚠️ 帧 {frame_idx} 大小不匹配，跳过")
                pbar.update(1)
                continue

            # 转灰度
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

            # 逐个 8x8 像素块读取
            for r in range(height // pixel_size):
                for c in range(width // pixel_size):
                    block = gray[r*pixel_size : (r+1)*pixel_size,
                                 c*pixel_size : (c+1)*pixel_size]
                    mean_val = np.mean(block)
                    bit = '1' if mean_val < 128 else '0'   # 黑色=1，白色=0（与原编码一致）
                    bits += bit

            pbar.update(1)

    cap.release()
    print(f"\n✅ 提取完成！共得到 {len(bits)} 个比特（约 {len(bits)//8} 字节）")

    # ==================== 关键步骤：每8位转字节 + XOR 0xAA ====================
    byte_data = bytearray()
    for i in range(0, len(bits) - (len(bits) % 8), 8):
        chunk = bits[i:i+8]
        byte_val = int(chunk, 2) ^ 0xAA          # XOR 还原（原编码用的 10101010 = 170）
        byte_data.append(byte_val)

    # 写入文件
    with open(output_file, "wb") as f:
        f.write(byte_data)

    print(f"🎉 已保存到 → {output_file}  (大小: {len(byte_data):,} 字节)")

    # ==================== 后续提示（自动判断文件类型） ====================
    print("\n" + "="*60)
    print("文件类型预览：")
    try:
        preview = byte_data[:200].decode("utf-8", errors="ignore")
        print("前200字节文本预览：")
        print(preview[:300])
        if "flag{" in preview.lower():
            print("🔥 发现 flag 特征！直接打开 extracted_payload.bin 查看")
    except:
        pass

    # 尝试判断常见文件类型
    header = bytes(byte_data[:8])
    if header.startswith(b'PK\x03\x04'):
        print("📦 这是一个 ZIP 文件 → 重命名为 flag.zip 后解压")
        os.rename(output_file, "flag.zip")
    elif header.startswith(b'\x89PNG'):
        print("🖼️ 这是一个 PNG 文件 → 重命名为 flag.png")
        os.rename(output_file, "flag.png")
    elif b"flag{" in header or b"flag{" in byte_data:
        print("🚩 发现 flag 字符串！直接用记事本/文本编辑器打开 extracted_payload.bin")

    print("\n解题完成！如果不是 flag 文件，请把 extracted_payload.bin 发给我，我再帮你继续分析下一层。")

if __name__ == "__main__":
    # ======== 你只需要修改下面这一行即可 ========
    extract_from_video(video_path="video.mp4")   # 如果你的视频文件名不同，请改这里
```

运行脚本得到bin文件，提示**每个哈希值对应一个字母**，解算后得到flag。
![逆向分析截图](@assets/uploads/note1/reverse1.jpg)
---

## Re3

### 解题流程
采用**流量数据提取 - 加密算法还原 - 密文解密验证**三步核心思路。

### 网络流量侧分析
1. 木马窃取数据传输目标端口：9999
2. 解析出3条JSON格式数据，对应文件：readme.txt、flag.txt、config.txt
3. flag.txt密文：`d0edd4a1620f6f01db93699e7291bc570b7d8cdd4fa0a69a0839ca4b86a7bd8daacd74313e64da169697af402033a761`
4. 无明文数据回传

### 加密算法特征判定
1. 分组密码，分组长度16字节，ECB模式，PKCS7类填充
2. 非标准算法，与SM4结构近似但参数自定义

### 二进制程序逆向
1. 含S盒查表、32位轮变换，24轮加密（区别于SM4标准32轮）
2. 轮函数为SM4风格T/T'函数，线性层为特定旋转组合
3. 核心常量为自定义值

### 密文解密结果
- readme.txt：常规说明文本
- config.txt：服务器基础配置
- flag.txt：`dart{f4b547fc-b3d0-44c3-bf21-8f3fb5ad3220}`

---

## Web题

1. 尝试admin弱口令爆破失败，注册普通用户仅可修改个人信息，角色为user
![逆向分析截图](@assets/uploads/note1/1.jpg)
2. 更换头像功能存在SSRF漏洞，`file:///etc/passwd`可读取系统文件，返回base64编码内容
![逆向分析截图](@assets/uploads/note1/2.jpg)
![逆向分析截图](@assets/uploads/note1/3.jpg)
3. 读取app.py源码，泄露Redis的Host、Port、Password
![逆向分析截图](@assets/uploads/note1/4.jpg)
4. 利用Redis EVAL命令获取`app:secret_key`：`9a5d6bbc594de8ab12ed12f68ee3261168ec5debdca941943b0a444c156b7eee`
5. 伪造role=admin的cookie提权，读取flag：`dart{ed597106-46ec-4344-8fea-d6cc7234ab5b}`

---

## Crypto题

### Level1
- 密钥对特征：key-1与key-2、key-4与key-15共因子；key-6/12/17为小私钥，适用Wiener攻击
![逆向分析截图](@assets/uploads/note1/5.jpg)
- 解密结果：
  - message2：Another success! One more cipher bites the dust!
  - message3：You're nearly there! Keep going! 3!
  - message4：You're nearly there! Keep going! 4!
  - message5：You're nearly there! Keep going! 5!
  - message6：You're nearly there! Keep going! 6!
  - message7：Congratulations! next pass is 9Zr4M1ThwVCHe4nHnmOcilJ8
  - message8：You're nearly there! Keep going! 8!
  - message9：You're nearly there! Keep going! 9!
  - message10：You're nearly there! Keep going! 10!
- level2.zip密码：`9Zr4M1ThwVCHe4nHnmOcilJ8`

### Level2
- d仅180位，使用Carmichael函数λ(n)而非φ(n)，适用Wiener攻击
- 修正因子：gcd(p-1, q-1)=4，恢复p+q
- level3.zip密码：`2aa9c360df99cbb4209e4dbab5a9f9ffd86d34906e3206fecfdabf0bb7aeb5ac`

### Level3
- n=p*q，leak为p、q位运算表达式
- 按模2^k逐位恢复p、q低位，唯一确定完整p、q
- 最终flag：`dart{379c9308-e9a8-45a1-bd55-45bbd822e86d}`

---

## 最终结果
- HYPERTYPHOON代表南开大学一队获得华北赛区第五十名，晋级复赛。


---

## 总结

本次CTF竞赛中，南开大学1队通过逆向工程、密码学分析、Web渗透等多种技术手段，成功解开了多个方向的题目。团队成员在短时间内快速分析题目特征，制定解题策略，展现了良好的技术能力和团队协作精神。

---
*最后更新：2026-03-14*
