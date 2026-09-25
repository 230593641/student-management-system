@echo off
chcp 65001 >nul
cd /d %~dp0
echo ============================================
echo   学生成绩管理系统（Web 版）启动中...
echo   浏览器访问: http://127.0.0.1:5000
echo   按 Ctrl+C 可停止服务
echo ============================================
start "" http://127.0.0.1:5000
python app.py
pause
