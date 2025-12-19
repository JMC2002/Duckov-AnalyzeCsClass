# ============================================
# Escape from Duckov DLL 结构导出工具
# 功能：
#  - 生成命名空间层级
#  - 提取 Class 信息
#  - 仅展示指定前缀（如 Duckov / Duckov.UI）
#  - 可控制展开层级（depth）
# ============================================

# --- 配置区域 ---

# DLL 所在的文件夹路径 (在此修改路径)
$baseDir = "U:\SteamLibrary\steamapps\common\Escape from Duckov\Duckov_Data\Managed"

# 输出目录
$outDir = ".\outputNamespace"
if (!(Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}

# 要分析的 DLL
$dllNames= @(
    "TeamSoda.Duckov.Utilities.dll",
    "TeamSoda.Duckov.Core.dll",
    "TeamSoda.MiniLocalizor.dll",
    "ItemStatsSystem.dll",
    "SodaLocalization.dll"
)

# 想看的命名空间前缀
$namespaceFilters = @("Duckov", "Duckov.UI", "Duckov.Utilities")

# 命名空间展开深度
$depth = 1

# --- 逻辑处理区域 ---

# 自动生成完整的目标路径列表
$targets = $dllNames | ForEach-Object { Join-Path $baseDir $_ }

foreach ($dll in $targets) {
    if (Test-Path $dll) {
        $fileName = [System.IO.Path]::GetFileNameWithoutExtension($dll)
        $outFile = Join-Path $outDir "$fileName.txt"
        $tempFile = "$outFile.tmp"

        Write-Host "正在分析：$fileName ..."

        # 提取
        ilspycmd -l cised $dll > $tempFile

        $lines = Get-Content $tempFile
        $results = New-Object System.Collections.Generic.List[string]
        $namespaces = New-Object System.Collections.Generic.HashSet[string]

        # 对过滤器按长度从长到短排序，确保优先匹配更精确的命名空间 (例如 Duckov.UI 优先于 Duckov)
        $sortedFilters = $namespaceFilters | Sort-Object { $_.Length } -Descending

        foreach ($line in $lines) {
            # 匹配 类 或 接口
            if ($line -match '^(Class|Interface)\s+([\w\.]+)') {
                $typeFullName = $matches[2]
                
                # 检查该类型是否符合任何一个过滤器
                foreach ($filter in $sortedFilters) {
                    # 确保是完全匹配前缀或以点分隔的前缀
                    if ($typeFullName -eq $filter -or $typeFullName.StartsWith("$filter.")) {
                        
                        $parts = $typeFullName.Split('.')
                        $filterPartsCount = $filter.Split('.').Length
                        
                        # 相对深度计算
                        # 例如: Filter=Duckov (1层), Depth=1 -> 允许显示到第 2 层 (Duckov.Name)
                        # 例如: Filter=Duckov.UI (2层), Depth=1 -> 允许显示到第 3 层 (Duckov.UI.Name)
                        $maxAllowedParts = $filterPartsCount + $depth

                        # 处理并记录命名空间层级
                        for ($i = $filterPartsCount; $i -lt $parts.Length; $i++) {
                            if ($i -le $maxAllowedParts) {
                                $ns = ($parts[0..($i-1)] -join '.')
                                if (-not $namespaces.Contains($ns)) {
                                    $namespaces.Add($ns) | Out-Null
                                    $results.Add("Namespace $ns")
                                }
                            }
                        }

                        # 如果当前类本身的层级在允许范围内，则添加该类
                        if ($parts.Length -le $maxAllowedParts) {
                            $results.Add($line)
                        }

                        # 既然已经找到了最精确的匹配并处理完毕，跳出当前行的过滤器循环
                        break 
                    }
                }
            }
        }

        # 输出
        # 注意：此处用 Sort-Object -Unique 保证 Namespace 和 Class 不重复
        $results | Sort-Object -Unique | Out-File $outFile -Encoding UTF8
        Remove-Item $tempFile -ErrorAction SilentlyContinue
    }
    else {
        Write-Warning "找不到文件：$dll"
    }
}

Write-Host ""
Write-Host "导出完成！结果保存在：$outDir"
