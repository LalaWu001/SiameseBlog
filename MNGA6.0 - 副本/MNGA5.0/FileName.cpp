#include <graphics.h>
#include <conio.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <string>
#include <windows.h>

// ��Ϸ�������� - �ӱ����ڴ�С
const int WINDOW_WIDTH = 520;  // ԭ����260���ӱ�
const int WINDOW_HEIGHT = 1000; // ԭ����500���ӱ�
const float GRAVITY = 0.5f;
const float FRICTION = 0.95f;
const float ELASTICITY = 0.3f;
const int FRUIT_TYPES = 8;
const float MERGE_DISTANCE = 1.2f;
const float BOTTOM_MARGIN = 120.0f; // ԭ����60���ӱ�
const float MIN_DISTANCE = 0.5f;
const float STABLE_THRESHOLD = 1.0f;
const float REST_VELOCITY_THRESHOLD = 0.5f;
const float COLLISION_RADIUS_FACTOR = 1.0f;

// ����ÿ��ˮ����ֱ�� - �ӱ�
const int FRUIT_DIAMETERS[FRUIT_TYPES] = {
    80,   // 0��ˮ��ֱ�� (ԭ����40)
    112,  // 1��ˮ��ֱ�� (ԭ����56)
    144,  // 2��ˮ��ֱ�� (ԭ����72)
    176,  // 3��ˮ��ֱ�� (ԭ����88)
    208,  // 4��ˮ��ֱ�� (ԭ����104)
    240,  // 5��ˮ��ֱ�� (ԭ����120)
    272,  // 6��ˮ��ֱ�� (ԭ����136)
    304   // 7��ˮ��ֱ�� (ԭ����152)
};

// ����Ԥ��ˮ����ֱ�� - �ӱ�
const int PREVIEW_DIAMETER = 80;  // ԭ����40

// ˮ��ͼƬ����
IMAGE fruitImages[FRUIT_TYPES];
IMAGE circleImages[FRUIT_TYPES]; // Բ�δ������ͼƬ

// ˮ����ɫ���� - ȫ����Ϊ��ɫ
COLORREF fruitColor = RGB(255, 255, 255); // ��ɫ����

// ˮ����
class Fruit {
public:
    float x, y;
    float vx, vy;
    float radius;
    float collisionRadius;
    int type;
    bool isStatic;
    bool isMerging;

    Fruit(float x, float y, int type) : x(x), y(y), type(type), isStatic(false), isMerging(false) {
        // ʹ��Ԥ�����ֱ�������ð뾶
        radius = static_cast<float>(FRUIT_DIAMETERS[type] / 2);
        collisionRadius = radius * COLLISION_RADIUS_FACTOR;
        vx = 0;
        vy = 0;
    }

    void update(std::vector<Fruit>& otherFruits) {
        if (!isStatic) {
            // Ӧ��������Ħ����
            vy += GRAVITY;
            vx *= FRICTION;
            vy *= FRICTION;

            // ����λ��
            x += vx;
            y += vy;

            // �߽���
            if (x - radius < 0) {
                x = radius;
                vx = -vx * ELASTICITY;
            }
            if (x + radius > WINDOW_WIDTH) {
                x = WINDOW_WIDTH - radius;
                vx = -vx * ELASTICITY;
            }
            if (y + radius > WINDOW_HEIGHT - BOTTOM_MARGIN) {
                y = WINDOW_HEIGHT - BOTTOM_MARGIN - radius;
                vy = -vy * ELASTICITY;

                // ����ٶȺ�С��ֱ�Ӿ�ֹ
                if (abs(vy) < REST_VELOCITY_THRESHOLD) {
                    vy = 0;
                    if (abs(vx) < REST_VELOCITY_THRESHOLD) {
                        isStatic = true;
                    }
                }
            }

            // ˮ��֮�����ײ������Ӧ
            for (auto& other : otherFruits) {
                if (&other != this && !other.isMerging && !isMerging) {
                    float dx = x - other.x;
                    float dy = y - other.y;
                    float distanceSquared = dx * dx + dy * dy;
                    float minDistance = collisionRadius + other.collisionRadius;

                    if (distanceSquared < minDistance * minDistance) {
                        float distance = sqrt(distanceSquared);
                        float overlap = minDistance - distance;

                        float nx = dx / distance;
                        float ny = dy / distance;

                        float separateX = nx * overlap * 0.5f;
                        float separateY = ny * overlap * 0.5f;

                        if (!isStatic && !other.isStatic) {
                            x += separateX;
                            y += separateY;
                            other.x -= separateX;
                            other.y -= separateY;

                            float vDotN = vx * nx + vy * ny;
                            float otherVDotN = other.vx * nx + other.vy * ny;

                            float v1After = otherVDotN * (1.0f + ELASTICITY);
                            float v2After = vDotN * (1.0f + ELASTICITY);

                            vx += (v1After - vDotN) * nx;
                            vy += (v1After - vDotN) * ny;
                            other.vx += (v2After - otherVDotN) * nx;
                            other.vy += (v2After - otherVDotN) * ny;
                        }
                        else if (!isStatic && other.isStatic) {
                            x += separateX * 2.0f;
                            y += separateY * 2.0f;

                            float vDotN = vx * nx + vy * ny;
                            vx -= (1.0f + ELASTICITY) * vDotN * nx;
                            vy -= (1.0f + ELASTICITY) * vDotN * ny;
                        }
                        else if (isStatic && !other.isStatic) {
                            other.x -= separateX * 2.0f;
                            other.y -= separateY * 2.0f;

                            float otherVDotN = other.vx * nx + other.vy * ny;
                            other.vx -= (1.0f + ELASTICITY) * otherVDotN * (-nx);
                            other.vy -= (1.0f + ELASTICITY) * otherVDotN * (-ny);
                        }
                    }
                }
            }

            // ����Ƿ���Ծ�ֹ
            if (abs(vx) < REST_VELOCITY_THRESHOLD && abs(vy) < REST_VELOCITY_THRESHOLD) {
                bool canRest = false;

                if (y + radius >= WINDOW_HEIGHT - BOTTOM_MARGIN - MIN_DISTANCE) {
                    canRest = true;
                }
                else {
                    for (auto& other : otherFruits) {
                        if (&other != this && !other.isMerging && !isMerging && other.isStatic) {
                            float dx = x - other.x;
                            float dy = y - other.y;
                            float distance = sqrt(dx * dx + dy * dy);

                            if (distance < collisionRadius + other.collisionRadius + STABLE_THRESHOLD) {
                                canRest = true;
                                break;
                            }
                        }
                    }
                }

                if (canRest) {
                    isStatic = true;
                    vx = 0;
                    vy = 0;
                }
            }
        }
    }

    void draw() const {
        if (!isMerging) {
            // 先绘制一个稍大的淡色圆形作为发光效果
            setfillcolor(RGB(240, 240, 255));
            solidcircle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(radius + 2));

            // 再绘制白色圆形背景
            setfillcolor(fruitColor);
            solidcircle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(radius));

            // 计算图片绘制位置
            int diameter = FRUIT_DIAMETERS[type];
            int imgX = static_cast<int>(x - diameter / 2);
            int imgY = static_cast<int>(y - diameter / 2);

            // 直接绘制预处理好的圆形图像
            putimage(imgX, imgY, &circleImages[type], SRCINVERT);

            // 绘制边框增强视觉效果
            setlinecolor(RGB(50, 50, 50));
            setlinestyle(PS_SOLID, 1);
            circle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(radius));
        }
    }

    void setUnstable() {
        if (isStatic) {
            isStatic = false;
            vx = 0;
            vy = 0.1f;
        }
    }

    bool hasSupport(const std::vector<Fruit>& otherFruits) const {
        if (y + radius >= WINDOW_HEIGHT - BOTTOM_MARGIN - MIN_DISTANCE) {
            return true;
        }

        for (const auto& other : otherFruits) {
            if (&other != this && !other.isMerging && other.isStatic) {
                float dx = x - other.x;
                float dy = y - other.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < collisionRadius + other.collisionRadius + STABLE_THRESHOLD && dy > 0) {
                    return true;
                }
            }
        }
        return false;
    }
};

// ��Ϸ��
class WatermelonMerge {
private:
    std::vector<Fruit> fruits;
    Fruit* currentFruit;
    bool isGameOver;
    bool isGameWin;
    int score;
    int nextFruitType;
    std::random_device rd;
    std::mt19937 gen;
    bool isWaitingForDrop;
    bool shouldSpawnNew;
    IMAGE previewImages[FRUIT_TYPES]; // Ԥ��ͼ��

public:
    WatermelonMerge() : isGameOver(false), isGameWin(false), score(0), gen(rd()), isWaitingForDrop(true), shouldSpawnNew(false), currentFruit(nullptr) {
        // ��ʼ��ˮ��ͼƬ
        loadFruitImages();
        spawnNewFruit();
    }

    // ����ˮ��ͼƬ������ΪԲ��
    void loadFruitImages() {
        // ���ظ���ˮ��PNGͼƬ
        loadimage(&fruitImages[0], _T("fruit/0.png"));
        loadimage(&fruitImages[1], _T("fruit/1.png"));
        loadimage(&fruitImages[2], _T("fruit/2.png"));
        loadimage(&fruitImages[3], _T("fruit/3.png"));
        loadimage(&fruitImages[4], _T("fruit/4.png"));
        loadimage(&fruitImages[5], _T("fruit/5.png"));
        loadimage(&fruitImages[6], _T("fruit/6.png"));
        loadimage(&fruitImages[7], _T("fruit/7.png"));

        // Ϊÿ��ˮ������Բ�δ���ͼ��
        for (int i = 0; i < FRUIT_TYPES; i++) {
            // ����Բ��ͼ��
            createCircleImage(i);

            // ����Ԥ��ͼ��
            createPreviewImage(i);
        }
    }

    // 创建圆形图像 - 改进裁剪精度
    void createCircleImage(int type) {
        int diameter = FRUIT_DIAMETERS[type];

        // 获取原始图片尺寸
        int origWidth = fruitImages[type].getwidth();
        int origHeight = fruitImages[type].getheight();

        if (origWidth <= 0 || origHeight <= 0) {
            return; // 图片未加载，退出
        }

        // 计算保持原始比例的新尺寸
        int newWidth, newHeight;
        if (origWidth >= origHeight) {
            newWidth = diameter;
            newHeight = static_cast<int>(origHeight * (static_cast<float>(diameter) / origWidth));
        }
        else {
            newHeight = diameter;
            newWidth = static_cast<int>(origWidth * (static_cast<float>(diameter) / origHeight));
        }

        // 创建缩放后的图像
        IMAGE resizedImg(diameter, diameter);

        // 设置工作图像
        SetWorkingImage(&resizedImg);
        setfillcolor(BLACK); // 黑色背景
        solidrectangle(0, 0, diameter, diameter);

        // 计算居中位置
        int offsetX = (diameter - newWidth) / 2;
        int offsetY = (diameter - newHeight) / 2;

        // 临时图像用于缩放
        IMAGE tempImg(newWidth, newHeight);
        SetWorkingImage(&tempImg);
        cleardevice();

        // 缩放原图到临时图像
        StretchBlt(GetImageHDC(&tempImg), 0, 0, newWidth, newHeight,
            GetImageHDC(&fruitImages[type]), 0, 0, origWidth, origHeight, SRCCOPY);

        // 把缩放后的图像放在中心位置
        SetWorkingImage(&resizedImg);
        putimage(offsetX, offsetY, &tempImg, SRCCOPY);

        // 创建掩码图像（白色背景，黑色圆形）
        IMAGE maskImg(diameter, diameter);
        SetWorkingImage(&maskImg);
        setfillcolor(WHITE);  // 白色背景
        solidrectangle(0, 0, diameter, diameter);
        setfillcolor(BLACK);  // 黑色圆形
        solidcircle(diameter / 2, diameter / 2, diameter / 2);
        SetWorkingImage(NULL);

        // 创建临时图像用于存储掩码处理结果
        IMAGE tempMasked(diameter, diameter);
        SetWorkingImage(&tempMasked);
        cleardevice();

        // 掩码处理：先应用掩码
        putimage(0, 0, &maskImg, SRCAND);
        // 然后叠加图像
        putimage(0, 0, &resizedImg, SRCPAINT);

        // 保存结果
        circleImages[type] = tempMasked;

        // 恢复工作图像
        SetWorkingImage(NULL);
    }

    // 创建预览图像
    void createPreviewImage(int type) {
        int diameter = PREVIEW_DIAMETER;

        // 获取原始图片尺寸
        int origWidth = fruitImages[type].getwidth();
        int origHeight = fruitImages[type].getheight();

        if (origWidth <= 0 || origHeight <= 0) {
            return; // 图片未加载，退出
        }

        // 计算保持原始比例的新尺寸
        int newWidth, newHeight;
        if (origWidth >= origHeight) {
            newWidth = diameter;
            newHeight = static_cast<int>(origHeight * (static_cast<float>(diameter) / origWidth));
        }
        else {
            newHeight = diameter;
            newWidth = static_cast<int>(origWidth * (static_cast<float>(diameter) / origHeight));
        }

        // 创建缩放后的图像
        IMAGE resizedImg(diameter, diameter);

        // 设置工作图像
        SetWorkingImage(&resizedImg);
        setfillcolor(BLACK); // 黑色背景
        solidrectangle(0, 0, diameter, diameter);

        // 计算居中位置
        int offsetX = (diameter - newWidth) / 2;
        int offsetY = (diameter - newHeight) / 2;

        // 临时图像用于缩放
        IMAGE tempImg(newWidth, newHeight);
        SetWorkingImage(&tempImg);
        cleardevice();

        // 缩放原图到临时图像
        StretchBlt(GetImageHDC(&tempImg), 0, 0, newWidth, newHeight,
            GetImageHDC(&fruitImages[type]), 0, 0, origWidth, origHeight, SRCCOPY);

        // 把缩放后的图像放在中心位置
        SetWorkingImage(&resizedImg);
        putimage(offsetX, offsetY, &tempImg, SRCCOPY);

        // 创建掩码图像（白色背景，黑色圆形）
        IMAGE maskImg(diameter, diameter);
        SetWorkingImage(&maskImg);
        setfillcolor(WHITE);  // 白色背景
        solidrectangle(0, 0, diameter, diameter);
        setfillcolor(BLACK);  // 黑色圆形
        solidcircle(diameter / 2, diameter / 2, diameter / 2);
        SetWorkingImage(NULL);

        // 创建临时图像用于存储掩码处理结果
        IMAGE tempMasked(diameter, diameter);
        SetWorkingImage(&tempMasked);
        cleardevice();

        // 掩码处理：先应用掩码
        putimage(0, 0, &maskImg, SRCAND);
        // 然后叠加图像
        putimage(0, 0, &resizedImg, SRCPAINT);

        // 保存结果
        previewImages[type] = tempMasked;

        // 恢复工作图像
        SetWorkingImage(NULL);
    }

    void spawnNewFruit() {
        std::uniform_int_distribution<> dis(0, 2);  // ֻ����ǰ����С��ˮ��
        nextFruitType = dis(gen);
        currentFruit = new Fruit(WINDOW_WIDTH / 2, 100, nextFruitType);  // ��߳�ʼλ��
        isWaitingForDrop = true;
        shouldSpawnNew = false;
    }

    void update() {
        if (currentFruit) {
            if (isWaitingForDrop) {
                // ��ȡ���λ��
                MOUSEMSG msg;
                if (MouseHit()) {
                    msg = GetMouseMsg();
                    currentFruit->x = static_cast<float>(msg.x);
                }
            }
            else {
                // ���µ�ǰˮ��
                currentFruit->update(fruits);
            }
        }

        for (auto& fruit : fruits) {
            fruit.update(fruits);
        }

        // ���ϲ�
        checkMerges();

        // �����Ϸ����
        checkGameOver();
    }

    void draw() {
        BeginBatchDraw();

        setbkcolor(RGB(239, 208, 252));
        cleardevice();

        // ���Ƶײ��߽�
        setfillcolor(RGB(143, 108, 184));
        solidrectangle(0, WINDOW_HEIGHT - BOTTOM_MARGIN, WINDOW_WIDTH, WINDOW_HEIGHT);

        // ���ƶ�������ָ���
        setlinecolor(RGB(180, 180, 180));
        setlinestyle(PS_SOLID, 2);
        line(0, 200, WINDOW_WIDTH, 200);  // �ӱ���Ķ�������

        // ��������ˮ��
        for (const auto& fruit : fruits) {
            fruit.draw();
        }

        // ���Ƶ�ǰ���Ƶ�ˮ��
        if (currentFruit && isWaitingForDrop) {
            // ���ư�͸���Ĺ켣��
            setlinestyle(PS_DASH, 1);
            setlinecolor(RGB(100, 100, 100));
            line(static_cast<int>(currentFruit->x), static_cast<int>(currentFruit->y),
                static_cast<int>(currentFruit->x), WINDOW_HEIGHT - BOTTOM_MARGIN);

            currentFruit->draw();
        }

        // ��ʾ��һ��ˮ��Ԥ��
        settextcolor(RGB(141, 86, 156));
        settextstyle(40, 0, _T("Impact"));  // �Ӵ�����
        outtextxy(WINDOW_WIDTH - 250, 20, _T("NEXT:"));

        // ������һ��ˮ��Ԥ��ͼ
        if (nextFruitType >= 0 && nextFruitType < FRUIT_TYPES) {
            // ����Ԥ��λ��
            int previewCenterX = WINDOW_WIDTH - 130;
            int previewCenterY = 60;

            // ���ư�ɫ��ɫ
            setfillcolor(fruitColor);
            solidcircle(previewCenterX, previewCenterY, PREVIEW_DIAMETER / 2);

            // ����Ԥ��ͼλ��
            int previewX = previewCenterX - PREVIEW_DIAMETER / 2;
            int previewY = previewCenterY - PREVIEW_DIAMETER / 2;

            // ����Բ��Ԥ��ͼ
            putimage(previewX, previewY, &previewImages[nextFruitType], SRCINVERT);

            // ���Ʊ߿�
            setlinecolor(RGB(50, 50, 50));
            setlinestyle(PS_SOLID, 1);
            circle(previewCenterX, previewCenterY, PREVIEW_DIAMETER / 2);
        }

        // ���Ʒ���
        TCHAR scoreText[32];
        _stprintf_s(scoreText, _T("Score: %d"), score);
        settextcolor(RGB(141, 86, 156));
        settextstyle(48, 0, _T("Impact"));  // �Ӵ�����
        outtextxy(20, 40, scoreText);

        // ������Ϸ������ʾ
        if (isGameOver) {
            settextcolor(RGB(141, 86, 156));
            settextstyle(64, 0, _T("Impact"));  // �Ӵ�����
            outtextxy(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2, _T("TRY AGAIN"));
        }
        else if (isGameWin) {
            settextcolor(RGB(141, 86, 156));
            settextstyle(64, 0, _T("Impact"));  // �Ӵ�����
            outtextxy(WINDOW_WIDTH / 2 - 225, WINDOW_HEIGHT / 2, _T("Make Nankai Great Againʼ"));
        }

        EndBatchDraw();
    }

    void handleInput() {
        if (isGameOver || isGameWin) {
            if (_kbhit()) {
                char key = _getch();
                if (key == 'r' || key == 'R') {
                    reset();
                }
            }
            return;
        }

        if (currentFruit && isWaitingForDrop) {
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_LBUTTONDOWN) {
                    isWaitingForDrop = false;
                    dropFruit();
                    spawnNewFruit();
                }
            }
        }
    }

    void dropFruit() {
        if (currentFruit) {
            currentFruit->vx = 0;
            fruits.push_back(*currentFruit);
            delete currentFruit;
            currentFruit = nullptr;
        }
    }

    void checkMerges() {
        std::vector<std::pair<size_t, size_t>> mergePairs;
        std::vector<size_t> mergedIndices;

        for (size_t i = 0; i < fruits.size(); i++) {
            for (size_t j = i + 1; j < fruits.size(); j++) {
                if (fruits[i].type == fruits[j].type && !fruits[i].isMerging && !fruits[j].isMerging) {
                    float dx = fruits[i].x - fruits[j].x;
                    float dy = fruits[i].y - fruits[j].y;
                    float distance = sqrt(dx * dx + dy * dy);
                    float mergeDistance = (fruits[i].radius + fruits[j].radius) * MERGE_DISTANCE;

                    if (distance < mergeDistance) {
                        mergePairs.push_back({ i, j });
                        mergedIndices.push_back(i);
                        mergedIndices.push_back(j);
                    }
                }
            }
        }

        for (const auto& pair : mergePairs) {
            size_t i = pair.first;
            size_t j = pair.second;

            if (!fruits[i].isMerging && !fruits[j].isMerging) {
                if (fruits[i].type < FRUIT_TYPES - 1) {
                    float newX = (fruits[i].x + fruits[j].x) / 2;
                    float newY = (fruits[i].y + fruits[j].y) / 2;
                    float newVx = (fruits[i].vx + fruits[j].vx) / 2;
                    float newVy = (fruits[i].vy + fruits[j].vy) / 2;

                    Fruit newFruit(newX, newY, fruits[i].type + 1);
                    newFruit.vx = newVx;
                    newFruit.vy = newVy;
                    fruits.push_back(newFruit);

                    score += (fruits[i].type + 1) * 10;

                    if (fruits[i].type + 1 == FRUIT_TYPES - 1) {
                        isGameWin = true;
                    }
                }
                fruits[i].isMerging = true;
                fruits[j].isMerging = true;
            }
        }

        if (!mergePairs.empty()) {
            fruits.erase(
                std::remove_if(fruits.begin(), fruits.end(),
                    [](const Fruit& f) { return f.isMerging; }),
                fruits.end()
            );

            for (auto& fruit : fruits) {
                if (fruit.isStatic && !fruit.hasSupport(fruits)) {
                    fruit.setUnstable();
                }
            }
        }
    }

    void checkGameOver() {
        for (const auto& fruit : fruits) {
            if (fruit.y - fruit.radius < 200 && fruit.isStatic) {  // �ӱ���Ķ�������
                isGameOver = true;
                break;
            }
        }
    }

    void reset() {
        fruits.clear();
        if (currentFruit) {
            delete currentFruit;
        }
        isGameOver = false;
        isGameWin = false;
        score = 0;
        shouldSpawnNew = false;
        spawnNewFruit();
    }
};

int main() {
    // ��ʼ��ͼ�δ���
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(RGB(238, 155, 255));
    cleardevice();

    // ��������Ϳ����
    LOGFONT f;
    gettextstyle(&f);
    f.lfQuality = ANTIALIASED_QUALITY;
    settextstyle(&f);

    // ������Ϸʵ��
    WatermelonMerge game;

    // ��Ϸ��ѭ��
    while (true) {
        game.handleInput();
        game.update();
        game.draw();
        Sleep(16);  // Լ60FPS
    }

    closegraph();
    return 0;
}