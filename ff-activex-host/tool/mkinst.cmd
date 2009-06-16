setlocal


cd install
"c:\Program Files\Inno Setup 5\iscc" /O../dist /dversion=%1 /dbasepath=../ ffactivex.iss

cd ..


endlocal
