@echo off
chcp 65001 >nul

echo ========================================
echo   SiameseBlog - Push to GitHub
echo ========================================
echo.

echo [1/4] Checking Git status...
git status
if %errorlevel% neq 0 (
    echo Error: Git command failed, please check if Git is installed
    pause
    exit /b 1
)
echo.

echo [2/4] Adding all changes to staging...
git add .
if %errorlevel% neq 0 (
    echo Error: Failed to add files
    pause
    exit /b 1
)
echo OK
echo.

echo [3/4] Committing changes...
set /p commit_msg="Enter commit message (press Enter for default): "
if "%commit_msg%"=="" set commit_msg=Update project
git commit -m "%commit_msg%"
if %errorlevel% neq 0 (
    echo Error: Commit failed
    pause
    exit /b 1
)
echo OK
echo.

echo [4/4] Pushing to GitHub...
git push
if %errorlevel% neq 0 (
    echo Error: Push failed
    pause
    exit /b 1
)
echo OK
echo.

echo ========================================
echo   Push completed!
echo ========================================
echo.
pause
