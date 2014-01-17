setlocal


cd install
"c:\Program Files\Inno Setup 5\iscc" /O../dist /dxversion=%1 /dxbasepath=../ ffactivex.iss

cd ..

copy dist\ffactivex-setup.exe dist\ffactivex-setup-%1.exe

endlocal
