@echo off
start /b cmd /k "timeout /t 2 /nobreak > nul & start http://localhost:3000"
npm run dev --prefix file_converter