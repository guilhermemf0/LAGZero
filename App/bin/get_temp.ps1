# get_temp.ps1
$temp = Get-CimInstance -ClassName Win32_PerfFormattedData_Counters_ThermalZone -Property CurrentReading
if ($temp) {
    # Converte de décimos de Kelvin para Celsius
    $celsius = ($temp.CurrentReading - 2732) / 10.0
    Write-Output $celsius
} else {
    Write-Output "Erro: Não foi possível obter a temperatura."
}