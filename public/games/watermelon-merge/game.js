// 游戏常量配置
const WINDOW_WIDTH = 520;
const WINDOW_HEIGHT = 1600;
const GRAVITY = 0.5;
const FRICTION = 0.95;
const ELASTICITY = 0.3;
const FRUIT_TYPES = 8;
const MERGE_DISTANCE = 1.2;
const BOTTOM_MARGIN = 80;
const MIN_DISTANCE = 0.5;
const STABLE_THRESHOLD = 1.0;
const REST_VELOCITY_THRESHOLD = 0.5;
const COLLISION_RADIUS_FACTOR = 1.0;

// 水果直径配置（缩小到原来的70%）
const FRUIT_DIAMETERS = [56, 78, 101, 123, 146, 168, 190, 213];
const PREVIEW_DIAMETER = 56;

// 水果颜色配置
const FRUIT_COLORS = [
    '#FF6B6B', // 0: 红色
    '#FF8C00', // 1: 橙色
    '#FFD700', // 2: 黄色
    '#32CD32', // 3: 绿色
    '#00CED1', // 4: 青色
    '#007BFF', // 5: 蓝色
    '#9B59B6', // 6: 紫色
    '#FF1493'  // 7: 粉色（最大水果）
];

// 游戏状态
let canvas, ctx;
let fruits = [];
let currentFruit = null;
let nextFruitType = 0;
let score = 0;
let isGameOver = false;
let isWaitingForDrop = true;
let mouseX = WINDOW_WIDTH / 2;
let mouseY = 100;
let gameWidth = WINDOW_WIDTH;
let gameHeight = WINDOW_HEIGHT;

// 水果类
class Fruit {
    constructor(x, y, type) {
        this.x = x;
        this.y = y;
        this.type = type;
        this.radius = FRUIT_DIAMETERS[type] / 2;
        this.collisionRadius = this.radius * COLLISION_RADIUS_FACTOR;
        this.vx = 0;
        this.vy = 0;
        this.isStatic = false;
        this.isMerging = false;
        this.color = FRUIT_COLORS[type];
    }

    update(otherFruits) {
        if (!this.isStatic) {
            // 应用重力
            this.vy += GRAVITY;
            this.vx *= FRICTION;
            this.vy *= FRICTION;

            // 多次迭代物理计算以提高稳定性
            const subSteps = 4;
            for (let step = 0; step < subSteps; step++) {
                // 更新位置
                this.x += this.vx / subSteps;
                this.y += this.vy / subSteps;

                // 边界碰撞
                if (this.x - this.radius < 0) {
                    this.x = this.radius;
                    this.vx = -this.vx * ELASTICITY;
                }
                if (this.x + this.radius > gameWidth) {
                    this.x = gameWidth - this.radius;
                    this.vx = -this.vx * ELASTICITY;
                }
                if (this.y + this.radius > gameHeight - BOTTOM_MARGIN) {
                    this.y = gameHeight - BOTTOM_MARGIN - this.radius;
                    this.vy = -this.vy * ELASTICITY;
                }

                // 水果之间的碰撞检测
                for (let other of otherFruits) {
                    if (other !== this && !other.isMerging && !this.isMerging) {
                        const dx = this.x - other.x;
                        const dy = this.y - other.y;
                        const distanceSquared = dx * dx + dy * dy;
                        const minDistance = this.collisionRadius + other.collisionRadius;

                        if (distanceSquared < minDistance * minDistance) {
                            const distance = Math.sqrt(distanceSquared);
                            const overlap = minDistance - distance;

                            const nx = dx / distance;
                            const ny = dy / distance;

                            // 分离重叠的水果
                            if (!this.isStatic && !other.isStatic) {
                                const separateX = nx * overlap * 0.5;
                                const separateY = ny * overlap * 0.5;
                                this.x += separateX;
                                this.y += separateY;
                                other.x -= separateX;
                                other.y -= separateY;

                                // 计算相对速度
                                const dvx = this.vx - other.vx;
                                const dvy = this.vy - other.vy;
                                const dvDotN = dvx * nx + dvy * ny;

                                // 只有当水果相互靠近时才处理碰撞
                                if (dvDotN < 0) {
                                    const restitution = ELASTICITY;
                                    const impulse = -(1 + restitution) * dvDotN / 2;
                                    this.vx += impulse * nx;
                                    this.vy += impulse * ny;
                                    other.vx -= impulse * nx;
                                    other.vy -= impulse * ny;
                                }
                            } else if (!this.isStatic && other.isStatic) {
                                const separateX = nx * overlap;
                                const separateY = ny * overlap;
                                this.x += separateX;
                                this.y += separateY;

                                const dvx = this.vx;
                                const dvy = this.vy;
                                const dvDotN = dvx * nx + dvy * ny;

                                if (dvDotN < 0) {
                                    const restitution = ELASTICITY;
                                    const impulse = -(1 + restitution) * dvDotN;
                                    this.vx += impulse * nx;
                                    this.vy += impulse * ny;
                                }
                            } else if (this.isStatic && !other.isStatic) {
                                const separateX = -nx * overlap;
                                const separateY = -ny * overlap;
                                other.x += separateX;
                                other.y += separateY;

                                const dvx = other.vx;
                                const dvy = other.vy;
                                const dvDotN = dvx * (-nx) + dvy * (-ny);

                                if (dvDotN < 0) {
                                    const restitution = ELASTICITY;
                                    const impulse = -(1 + restitution) * dvDotN;
                                    other.vx += impulse * (-nx);
                                    other.vy += impulse * (-ny);
                                }
                            }
                        }
                    }
                }
            }

            // 检查是否可以静止
            if (Math.abs(this.vx) < REST_VELOCITY_THRESHOLD && Math.abs(this.vy) < REST_VELOCITY_THRESHOLD) {
                let canRest = false;

                if (this.y + this.radius >= gameHeight - BOTTOM_MARGIN - MIN_DISTANCE) {
                    canRest = true;
                } else {
                    for (let other of otherFruits) {
                        if (other !== this && !other.isMerging && !this.isMerging && other.isStatic) {
                            const dx = this.x - other.x;
                            const dy = this.y - other.y;
                            const distance = Math.sqrt(dx * dx + dy * dy);

                            if (distance < this.collisionRadius + other.collisionRadius + STABLE_THRESHOLD && dy > 0) {
                                canRest = true;
                                break;
                            }
                        }
                    }
                }

                if (canRest) {
                    this.isStatic = true;
                    this.vx = 0;
                    this.vy = 0;
                }
            }
        }
    }

    draw(ctx) {
        if (!this.isMerging) {
            // 绘制发光效果
            ctx.beginPath();
            ctx.arc(this.x, this.y, this.radius + 2, 0, Math.PI * 2);
            ctx.fillStyle = 'rgba(240, 240, 255, 0.3)';
            ctx.fill();

            // 绘制水果主体
            ctx.beginPath();
            ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
            ctx.fillStyle = this.color;
            ctx.fill();

            // 绘制边框
            ctx.beginPath();
            ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
            ctx.strokeStyle = 'rgba(50, 50, 50, 0.5)';
            ctx.lineWidth = 2;
            ctx.stroke();
        }
    }

    setUnstable() {
        if (this.isStatic) {
            this.isStatic = false;
            this.vx = 0;
            this.vy = 0.1;
        }
    }

    hasSupport(otherFruits) {
        if (this.y + this.radius >= gameHeight - BOTTOM_MARGIN - MIN_DISTANCE) {
            return true;
        }

        for (let other of otherFruits) {
            if (other !== this && !other.isMerging && !this.isMerging && other.isStatic) {
                const dx = this.x - other.x;
                const dy = this.y - other.y;
                const distance = Math.sqrt(dx * dx + dy * dy);

                if (distance < this.collisionRadius + other.collisionRadius + STABLE_THRESHOLD && dy > 0) {
                    return true;
                }
            }
        }
        return false;
    }
}

// 游戏初始化
function init() {
    canvas = document.getElementById('game-canvas');
    ctx = canvas.getContext('2d');
    
    // 设置画布大小
    resizeCanvas();
    window.addEventListener('resize', resizeCanvas);

    // 添加事件监听
    canvas.addEventListener('mousemove', handleMouseMove);
    canvas.addEventListener('click', handleClick);
    canvas.addEventListener('touchmove', handleTouchMove, { passive: false });
    canvas.addEventListener('touchstart', handleClick, { passive: false });

    // 生成第一个水果
    spawnNewFruit();

    // 开始游戏循环
    gameLoop();
}

// 调整画布大小
function resizeCanvas() {
    const container = document.getElementById('game-container');
    gameWidth = Math.min(WINDOW_WIDTH, window.innerWidth - 40);
    gameHeight = Math.min(WINDOW_HEIGHT, window.innerHeight - 40);
    
    canvas.width = gameWidth;
    canvas.height = gameHeight;
}

// 生成新水果
function spawnNewFruit() {
    nextFruitType = Math.floor(Math.random() * 3); // 0-2: 小水果
    currentFruit = new Fruit(gameWidth / 2, 100, nextFruitType);
    isWaitingForDrop = true;
}

// 投放水果
function dropFruit() {
    if (currentFruit) {
        currentFruit.vx = 0;
        fruits.push(currentFruit);
        currentFruit = null;
        spawnNewFruit();
    }
}

// 检查水果合并
function checkMerges() {
    const mergePairs = [];
    const mergedIndices = new Set();

    for (let i = 0; i < fruits.length; i++) {
        for (let j = i + 1; j < fruits.length; j++) {
            if (fruits[i].type === fruits[j].type && 
                !fruits[i].isMerging && 
                !fruits[j].isMerging &&
                !mergedIndices.has(i) && 
                !mergedIndices.has(j)) {
                
                const dx = fruits[i].x - fruits[j].x;
                const dy = fruits[i].y - fruits[j].y;
                const distance = Math.sqrt(dx * dx + dy * dy);

                if (distance < MERGE_DISTANCE * FRUIT_DIAMETERS[fruits[i].type]) {
                    mergePairs.push([i, j]);
                    mergedIndices.add(i);
                    mergedIndices.add(j);
                }
            }
        }
    }

    // 执行合并
    for (const [i, j] of mergePairs) {
        mergeFruits(i, j);
    }
}

// 合并两个水果
function mergeFruits(index1, index2) {
    const fruit1 = fruits[index1];
    const fruit2 = fruits[index2];

    if (!fruit1 || !fruit2) return;

    // 标记为正在合并
    fruit1.isMerging = true;
    fruit2.isMerging = true;

    // 计算新水果位置（中点）
    const newX = (fruit1.x + fruit2.x) / 2;
    const newY = (fruit1.y + fruit2.y) / 2;

    // 计算新类型
    const newType = fruit1.type + 1;

    // 检查是否可以合并
    if (newType >= FRUIT_TYPES) {
        // 游戏胜利
        isGameOver = true;
        return;
    }

    // 移除旧水果
    fruits.splice(Math.max(index1, index2), 1);
    fruits.splice(Math.min(index1, index2), 1);

    // 创建新水果
    const newFruit = new Fruit(newX, newY, newType);
    newFruit.isMerging = false;
    newFruit.isStatic = false;
    newFruit.vy = 0.5; // 给一个小的初始速度，确保它会下落
    fruits.push(newFruit);

    // 检查是否有其他水果失去了支撑
    for (let fruit of fruits) {
        if (fruit.isStatic && !fruit.hasSupport(fruits)) {
            fruit.setUnstable();
        }
    }

    // 更新分数
    score += (newType + 1) * 10;
    updateScore();

    // 检查游戏结束
    checkGameOver();
}

// 检查游戏结束
function checkGameOver() {
    if (isGameOver) {
        document.getElementById('game-over').style.display = 'block';
        return;
    }

    // 检查是否超过顶部线（200px）
    // 只检查已经投放的水果（排除当前等待投放的水果）
    for (let fruit of fruits) {
        if (fruit.y - fruit.radius < 200) {
            // 如果水果已经稳定（速度很小）且不是刚投放的，则游戏结束
            if (Math.abs(fruit.vx) < REST_VELOCITY_THRESHOLD && 
                Math.abs(fruit.vy) < REST_VELOCITY_THRESHOLD &&
                fruit.y > 150) { // 确保水果已经下落了一段时间
                isGameOver = true;
                document.getElementById('game-over').style.display = 'block';
                return;
            }
        }
    }
}

// 更新分数显示
function updateScore() {
    document.getElementById('score').textContent = `Score: ${score}`;
}

// 绘制预览
function drawPreview() {
    const previewCanvas = document.getElementById('preview-canvas');
    const previewCtx = previewCanvas.getContext('2d');
    
    previewCtx.clearRect(0, 0, previewCanvas.width, previewCanvas.height);
    
    // 绘制预览水果
    const radius = PREVIEW_DIAMETER / 2;
    
    // 绘制发光效果
    previewCtx.beginPath();
    previewCtx.arc(previewCanvas.width / 2, previewCanvas.height / 2, radius + 2, 0, Math.PI * 2);
    previewCtx.fillStyle = 'rgba(240, 240, 255, 0.3)';
    previewCtx.fill();

    // 绘制水果主体
    previewCtx.beginPath();
    previewCtx.arc(previewCanvas.width / 2, previewCanvas.height / 2, radius, 0, Math.PI * 2);
    previewCtx.fillStyle = FRUIT_COLORS[nextFruitType];
    previewCtx.fill();

    // 绘制边框
    previewCtx.beginPath();
    previewCtx.arc(previewCanvas.width / 2, previewCanvas.height / 2, radius, 0, Math.PI * 2);
    previewCtx.strokeStyle = 'rgba(50, 50, 50, 0.5)';
    previewCtx.lineWidth = 2;
    previewCtx.stroke();
}

// 绘制游戏
function draw() {
    // 清空画布
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // 绘制背景
    ctx.fillStyle = 'rgba(143, 108, 184, 0.8)';
    ctx.fillRect(0, gameHeight - BOTTOM_MARGIN, canvas.width, BOTTOM_MARGIN);

    // 绘制分隔线
    ctx.strokeStyle = 'rgba(180, 180, 180, 0.8)';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(0, 200);
    ctx.lineTo(canvas.width, 200);
    ctx.stroke();

    // 绘制所有水果
    for (let fruit of fruits) {
        fruit.draw(ctx);
    }

    // 绘制当前水果
    if (currentFruit && isWaitingForDrop) {
        // 绘制虚线
        ctx.strokeStyle = 'rgba(100, 100, 100, 0.8)';
        ctx.lineWidth = 1;
        ctx.setLineDash([5, 5]);
        ctx.beginPath();
        ctx.moveTo(currentFruit.x, currentFruit.y);
        ctx.lineTo(currentFruit.x, gameHeight - BOTTOM_MARGIN);
        ctx.stroke();
        ctx.setLineDash([]);

        currentFruit.draw(ctx);
    }

    // 绘制预览
    drawPreview();
}

// 游戏循环
function gameLoop() {
    if (!isGameOver) {
        // 更新水果
        if (currentFruit) {
            if (isWaitingForDrop) {
                currentFruit.x = mouseX;
            } else {
                currentFruit.update(fruits);
            }
        }

        for (let fruit of fruits) {
            fruit.update(fruits);
        }

        // 检查合并
        checkMerges();

        // 检查游戏结束
        checkGameOver();
    }

    // 绘制
    draw();

    requestAnimationFrame(gameLoop);
}

// 处理鼠标移动
function handleMouseMove(e) {
    const rect = canvas.getBoundingClientRect();
    mouseX = e.clientX - rect.left;
    mouseY = e.clientY - rect.top;
}

// 处理触摸移动
function handleTouchMove(e) {
    e.preventDefault();
    const rect = canvas.getBoundingClientRect();
    mouseX = e.touches[0].clientX - rect.left;
    mouseY = e.touches[0].clientY - rect.top;
}

// 处理点击
function handleClick(e) {
    if (isGameOver) return;

    if (currentFruit && isWaitingForDrop) {
        isWaitingForDrop = false;
        dropFruit();
    }
}

// 重置游戏
function resetGame() {
    fruits = [];
    score = 0;
    isGameOver = false;
    isWaitingForDrop = true;
    currentFruit = null;
    updateScore();
    document.getElementById('game-over').style.display = 'none';
    spawnNewFruit();
}

// 页面加载完成后初始化
window.onload = init;
