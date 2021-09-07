@echo off
::删除当前目录下文件夹的所有build文件夹，不包含子目录（只删除一级目录中的build，方便清理工程后Git）
for /f "delims=" %%a in ('dir /ad/b .\') do (rd /q /s "%%a\build")
pause