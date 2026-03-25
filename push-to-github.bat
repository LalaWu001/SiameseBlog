@echo off
chcp 65001 >nul

echo ========================================
echo   SiameseBlog - 一键推送到 GitHub
echo ========================================
echo.

echo [1/4] 检查 Git 状态...
git status
if %errorlevel% neq 0 (
    echo 错误：Git 命令失败，请检查是否安装了 Git
    pause
    exit /b 1
)
echo.

echo [2/4] 添加所有更改到暂存区...
git add .
if %errorlevel% neq 0 (
    echo 错误：添加文件失败
    pause
    pause
    exit /b 1
)
echo ✓ 文件添加成功
echo.

echo [3/4] 提交更改...
set /p commit_msg="请输入提交信息（按回车使用默认信息）: "
if "%commit_msg%"=="" set commit_msg=更新项目
git commit -m "%commit_msg%"
if %errorlevel% neq 0 (
    echo 错误：提交失败
    pause
    exit /b 1
)
echo ✓ 提交成功
echo.

echo [4/4] 推送到 GitHub...
git push
if %errorlevel% neq 0 (
    echo 错误：推送失败
    pause
    exit /b 1
)
echo ✓ 推送成功
echo.

echo ========================================
echo   推送完成！
echo ========================================
echo.
echo 你的更改已成功推送到 GitHub。
echo Vercel 将自动重新部署你的网站。
echo.
pause
