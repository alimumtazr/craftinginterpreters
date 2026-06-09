<#
  Chapterwise test runner for the Lox interpreter.

  Each .lox test file is annotated with expected results in comments:
    // expect: TEXT          -> TEXT must appear as the next line of stdout (in order)
    // expect error: TEXT    -> TEXT must appear somewhere in stderr (substring)

  Usage (from anywhere):
    powershell -ExecutionPolicy Bypass -File tests\run_tests.ps1
    powershell -ExecutionPolicy Bypass -File tests\run_tests.ps1 -NoBuild
#>

param(
  [switch]$NoBuild
)

$ErrorActionPreference = "Stop"
$testsDir = $PSScriptRoot
$root = Split-Path -Parent $testsDir
$exe = Join-Path $testsDir "lox_test.exe"

$sources = @(
  "lox.cpp", "scanner.cpp", "Parser.cpp", "Interpreter.cpp",
  "LoxFunction.cpp", "Resolver.cpp", "LoxClass.cpp", "LoxInstance.cpp"
) | ForEach-Object { Join-Path $root $_ }

if (-not $NoBuild) {
  Write-Host "Building interpreter..." -ForegroundColor Cyan
  & g++ -std=c++17 -o $exe @sources
  if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed." -ForegroundColor Red
    exit 1
  }
}

if (-not (Test-Path $exe)) {
  Write-Host "Interpreter not found: $exe (run without -NoBuild first)" -ForegroundColor Red
  exit 1
}

$tests = Get-ChildItem -Path $testsDir -Recurse -Filter *.lox | Sort-Object FullName
$pass = 0
$fail = 0

foreach ($test in $tests) {
  $lines = Get-Content -Path $test.FullName
  $expectOut = @()
  $expectErr = @()
  foreach ($line in $lines) {
    if ($line -match '//\s*expect error:\s?(.*)$') {
      $expectErr += $Matches[1].TrimEnd()
    } elseif ($line -match '//\s*expect:\s?(.*)$') {
      $expectOut += $Matches[1].TrimEnd()
    }
  }

  $outFile = [System.IO.Path]::GetTempFileName()
  $errFile = [System.IO.Path]::GetTempFileName()
  Start-Process -FilePath $exe -ArgumentList "`"$($test.FullName)`"" `
    -NoNewWindow -Wait -PassThru `
    -RedirectStandardOutput $outFile -RedirectStandardError $errFile | Out-Null

  $actualOut = @(Get-Content -Path $outFile)
  $actualErr = (Get-Content -Path $errFile -Raw)
  if ($null -eq $actualErr) { $actualErr = "" }
  Remove-Item $outFile, $errFile -ErrorAction SilentlyContinue

  # Drop trailing blank lines from stdout.
  $last = $actualOut.Count - 1
  while ($last -ge 0 -and $actualOut[$last] -eq "") { $last-- }
  if ($last -lt 0) { $actualOut = @() } else { $actualOut = @($actualOut[0..$last]) }

  $ok = $true
  $reason = @()

  if ($actualOut.Count -ne $expectOut.Count) {
    $ok = $false
    $reason += "stdout: expected $($expectOut.Count) line(s), got $($actualOut.Count)"
  } else {
    for ($i = 0; $i -lt $expectOut.Count; $i++) {
      if ($actualOut[$i] -ne $expectOut[$i]) {
        $ok = $false
        $reason += "line $($i + 1): expected [$($expectOut[$i])], got [$($actualOut[$i])]"
      }
    }
  }

  foreach ($e in $expectErr) {
    if ($actualErr -notmatch [regex]::Escape($e)) {
      $ok = $false
      $reason += "missing error: [$e]"
    }
  }

  $rel = $test.FullName.Substring($testsDir.Length + 1)
  if ($ok) {
    $pass++
    Write-Host "PASS  $rel" -ForegroundColor Green
  } else {
    $fail++
    Write-Host "FAIL  $rel" -ForegroundColor Red
    foreach ($r in $reason) { Write-Host "        $r" -ForegroundColor Yellow }
  }
}

Write-Host ""
Write-Host "Passed: $pass   Failed: $fail   Total: $($pass + $fail)" `
  -ForegroundColor ($(if ($fail -gt 0) { "Red" } else { "Green" }))

if ($fail -gt 0) { exit 1 } else { exit 0 }
