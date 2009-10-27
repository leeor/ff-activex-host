setlocal


cd install
"c:\Program Files\Inno Setup 5\iscc" /O../dist /dxversion=%1 /dxbasepath=../ ffactivex.iss

cd ..


endlocal
