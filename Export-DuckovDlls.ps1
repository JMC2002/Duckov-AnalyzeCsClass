# ============================================
# Escape from Duckov DLL 结构导出工具
# 功能：
#  - 生成命名空间层级
#  - 提取 Class 信息
#  - 仅展示指定前缀（如 Duckov / Duckov.UI）
#  - 可控制展开层级（depth）
# ============================================

# 输出目录
$outDir = ".\output"
if (!(Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}

# 要分析的 DLL
$targets = @(
    "D:\SteamLibrary\steamapps\common\Escape from Duckov\Duckov_Data\Managed\TeamSoda.Duckov.Utilities.dll",
    "D:\SteamLibrary\steamapps\common\Escape from Duckov\Duckov_Data\Managed\TeamSoda.Duckov.Core.dll",
    "D:\SteamLibrary\steamapps\common\Escape from Duckov\Duckov_Data\Managed\TeamSoda.MiniLocalizor.dll",
    "D:\SteamLibrary\steamapps\common\Escape from Duckov\Duckov_Data\Managed\ItemStatsSystem.dll",
    "D:\SteamLibrary\steamapps\common\Escape from Duckov\Duckov_Data\Managed\SodaLocalization.dll"
)

# 想看的命名空间前缀
$namespaceFilters = @("Duckov")

# 命名空间展开深度
$depth = 2
foreach ($dll in $targets) {
    if (Test-Path $dll) {
        $fileName = [System.IO.Path]::GetFileNameWithoutExtension($dll)
        $outFile = Join-Path $outDir "$fileName.txt"
        $tempFile = "$outFile.tmp"

        Write-Host "正在分析：$fileName ..."

        # =============================
        # 阶段一：提取类 / 接口 / 命名空间
        # =============================
        ilspycmd -l cised $dll > $tempFile

        $lines = Get-Content $tempFile
        $results = New-Object System.Collections.Generic.List[string]
        $namespaces = New-Object System.Collections.Generic.HashSet[string]

foreach ($line in $lines) {
    if ($line -match '^(Class|Interface)\s+([\w\.]+)') {
        $typeFullName = $matches[2]

        foreach ($filter in $namespaceFilters) {
            if ($typeFullName -like "$filter*") {
                # 拆分命名空间层级
                $parts = $typeFullName.Split('.')

                # 计算最大显示层级（相对前缀）
                $filterDepth = $filter.Split('.').Length
                $maxDepth = $filterDepth + $depth - 1
                $maxParts = [Math]::Min($parts.Length, $maxDepth)

                # 记录命名空间层级（受depth限制）
                for ($i = $filterDepth; $i -lt $maxParts; $i++) {
                    $ns = ($parts[0..$i] -join '.')
                    if (-not $namespaces.Contains($ns)) {
                        $namespaces.Add($ns) | Out-Null
                        $results.Add("Namespace $ns")
                    }
                }

                # 仅当类深度 <= 允许深度时才输出类
                if ($parts.Length -le $maxDepth) {
                    $results.Add($line)
                }

                break
            }
        }
    }
}

        # =============================
        # 输出
        # =============================
        $results | Sort-Object -Unique | Out-File $outFile -Encoding UTF8
        Remove-Item $tempFile -ErrorAction SilentlyContinue
    }
    else {
        Write-Warning "找不到文件：$dll"
    }
}

Write-Host ""
Write-Host "导出完成！结果保存在：$outDir"
