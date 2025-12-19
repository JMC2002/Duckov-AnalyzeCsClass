# ============================================
# DLL 命名空间结构探测工具
# 功能：快速列出 DLL 中存在的所有命名空间（不看类）
# ============================================

# --- 配置区域 ---
$baseDir = "U:\SteamLibrary\steamapps\common\Escape from Duckov\Duckov_Data\Managed"
$outDir = ".\outputRootNamespace"

# 要分析的 DLL
$dllNames = @(
    "TeamSoda.Duckov.Utilities.dll",
    "TeamSoda.Duckov.Core.dll",
    "TeamSoda.MiniLocalizor.dll",
    "ItemStatsSystem.dll",
    "SodaLocalization.dll"
)

# 探测深度：1表示只看第一级 (如 TeamSoda), 2表示看两级 (如 TeamSoda.Duckov)
$discoveryDepth = 1

# --- 逻辑处理区域 ---
if (!(Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }

foreach ($dllName in $dllNames) {
    $dllPath = Join-Path $baseDir $dllName
    if (Test-Path $dllPath) {
        Write-Host "正在探测：$dllName ..." -ForegroundColor Cyan
        
        $fileName = [System.IO.Path]::GetFileNameWithoutExtension($dllName)
        $outFile = Join-Path $outDir "$fileName.txt"
        $namespaces = New-Object System.Collections.Generic.HashSet[string]

        # 调用 ilspycmd 提取类型列表
        $output = ilspycmd -l cised $dllPath

        foreach ($line in $output) {
            # 匹配 Class, Interface, Struct, Enum 等
            if ($line -match '^(Class|Interface|Struct|Enum|ValueType)\s+([\w\.]+)') {
                $fullName = $matches[2]
                
                # 提取命名空间（去掉最后一个类名部分）
                if ($fullName.Contains(".")) {
                    $nsFull = $fullName.Substring(0, $fullName.LastIndexOf('.'))
                    
                    # 根据定义的深度截取根命名空间
                    $parts = $nsFull.Split('.')
                    $displayDepth = [Math]::Min($parts.Length, $discoveryDepth)
                    $rootNs = $parts[0..($displayDepth-1)] -join '.'
                    
                    $namespaces.Add($rootNs) | Out-Null
                }
                else {
                    # 如果类没有命名空间，归类为 <Global>
                    $namespaces.Add("<Global Namespace>") | Out-Null
                }
            }
        }

        # 排序并输出
        $namespaces | Sort-Object | Out-File $outFile -Encoding UTF8
        Write-Host "  - 发现 $($namespaces.Count) 个根路径，已保存至 $outFile"
    }
    else {
        Write-Warning "找不到文件：$dllPath"
    }
}

Write-Host "`n全部探测完成！请查看 $outDir 目录。" -ForegroundColor Green