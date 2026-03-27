    // 加载水果图片
    void loadFruitImages() {
        // 加载所有水果图片
        loadimage(&fruitImages[0], _T("fruit/0.png"));
        loadimage(&fruitImages[1], _T("fruit/1.png"));
        loadimage(&fruitImages[2], _T("fruit/2.png"));
        loadimage(&fruitImages[3], _T("fruit/3.png"));
        loadimage(&fruitImages[4], _T("fruit/4.png"));
        loadimage(&fruitImages[5], _T("fruit/5.png"));
        loadimage(&fruitImages[6], _T("fruit/6.png"));
        loadimage(&fruitImages[7], _T("fruit/7.png"));

        // 处理每个图片和掩码
        for (int i = 0; i < FRUIT_TYPES; i++) {
            int diameter = static_cast<int>(FRUIT_RADIUSES[i] * 2);  // 使用预设半径计算直径
            
            // 根据原始图片大小进行缩放，保持比例
            int origWidth = fruitImages[i].getwidth();
            int origHeight = fruitImages[i].getheight();
            
            if (origWidth > 0 && origHeight > 0) {
                // 计算保持原始比例的新尺寸
                int newWidth, newHeight;
                if (origWidth >= origHeight) {
                    newWidth = diameter;
                    newHeight = static_cast<int>(origHeight * (static_cast<float>(diameter) / origWidth));
                } else {
                    newHeight = diameter;
                    newWidth = static_cast<int>(origWidth * (static_cast<float>(diameter) / origHeight));
                }
                
                // 创建缩放后的图像
                IMAGE scaledImg;
                scaledImg.Resize(diameter, diameter);
                SetWorkingImage(&scaledImg);
                setfillcolor(BLACK);  // 用黑色填充背景
                solidrectangle(0, 0, diameter, diameter);
                
                // 计算图像居中位置
                int offsetX = (diameter - newWidth) / 2;
                int offsetY = (diameter - newHeight) / 2;
                
                // 缩放图像
                IMAGE tempImg;
                tempImg.Resize(newWidth, newHeight);
                SetWorkingImage(&tempImg);
                StretchBlt(GetImageHDC(&tempImg), 0, 0, newWidth, newHeight,
                    GetImageHDC(&fruitImages[i]), 0, 0, origWidth, origHeight, SRCCOPY);
                
                // 将缩放后的图像居中放置
                SetWorkingImage(&scaledImg);
                putimage(offsetX, offsetY, &tempImg, SRCCOPY);
                
                SetWorkingImage(NULL);
                fruitImages[i] = scaledImg;
                
                // 创建掩码图像
                fruitMasks[i].Resize(diameter, diameter);
                SetWorkingImage(&fruitMasks[i]);
                // 掩码图像为白色背景
                setfillcolor(WHITE);
                solidrectangle(0, 0, diameter, diameter);
                // 绘制黑色圆形（这样与PNG的透明区域配合使用）
                setfillcolor(BLACK);
                solidcircle(diameter/2, diameter/2, diameter/2);
                
                SetWorkingImage(NULL);
            }
        }
    }
    
    void spawnNewFruit() {
        std::uniform_int_distribution<> dis(0, 2);  // 只生成前三种水果
        nextFruitType = dis(gen);
        currentFruit = new Fruit(WINDOW_WIDTH / 2, 80, nextFruitType);  // 调整初始高度
        isWaitingForDrop = true;
        shouldSpawnNew = false;
    }