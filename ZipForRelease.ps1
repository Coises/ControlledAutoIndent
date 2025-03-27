$Project = "ControlledAutoIndent"
$x86Version = (Get-Command ".vs\bin\Win32\Release\$Project.dll").FileVersionInfo.ProductVersion
Compress-Archive -DestinationPath "$Project-$x86Version-x86.zip" -Path ".vs\bin\Win32\Release\*"
$x64Version = (Get-Command ".vs\bin\x64\Release\$Project.dll").FileVersionInfo.ProductVersion
Compress-Archive -DestinationPath "$Project-$x64Version-x64.zip" -Path ".vs\bin\x64\Release\*"
$x86Hash = (Get-FileHash "$Project-$x86Version-x86.zip" -Algorithm SHA256).Hash
$x64Hash = (Get-FileHash "$Project-$x64Version-x64.zip" -Algorithm SHA256).Hash
$Hashes =  "SHA-256 Hash ($Project-$x64Version-x64.zip): " + $x64Hash + "`nSHA-256 Hash ($Project-$x86Version-x86.zip): " + $x86Hash
Write-Output $Hashes
Write-Output ""
Set-Clipboard -Value $Hashes
Read-Host -Prompt "Press Enter to exit"
