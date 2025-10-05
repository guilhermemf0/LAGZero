using System;
using System.Threading;
using System.Globalization;
using LibreHardwareMonitor.Hardware;
using System.Text;
using System.Linq;
using System.Collections.Generic;
using System.Management;

public class UpdateVisitor : IVisitor
{
    public void VisitComputer(IComputer computer) { computer.Traverse(this); }
    public void VisitHardware(IHardware hardware)
    {
        hardware.Update();
        foreach (IHardware subHardware in hardware.SubHardware) subHardware.Accept(this);
    }
    public void VisitSensor(ISensor sensor) { }
    public void VisitParameter(IParameter parameter) { }
}

public class HardwareMonitor
{
    public static void Main()
    {
        StringBuilder result = new StringBuilder();

        try
        {
            Computer computer = new Computer
            {
                IsCpuEnabled = true,
                IsGpuEnabled = true,
                IsMotherboardEnabled = true,
                IsStorageEnabled = true,
                IsMemoryEnabled = true
            };

            computer.Open();
            Thread.Sleep(1000);
            computer.Accept(new UpdateVisitor());

            foreach (IHardware hardware in computer.Hardware)
            {
                hardware.Update();
                string hardwareIdentifier = GetHardwareIdentifier(hardware);
                string hardwareName = hardware.Name.Replace(":", "").Replace(";", "");

                // Lógica corrigida para garantir que componentes principais sem sensor de temp. sejam reportados
                if (hardware.HardwareType == HardwareType.Cpu || 
                    hardware.HardwareType == HardwareType.GpuAmd || 
                    hardware.HardwareType == HardwareType.GpuNvidia || 
                    hardware.HardwareType == HardwareType.GpuIntel || // <-- Adicionado GPU Intel
                    hardware.HardwareType == HardwareType.Motherboard)
                {
                    bool hasValidTempSensor = hardware.Sensors.Any(s => s.SensorType == SensorType.Temperature && s.Value.HasValue);
                    if (!hasValidTempSensor)
                    {
                         result.AppendFormat("{0}_TEMPERATURE_0:{1}:No Sensor:-1;",
                            hardwareIdentifier,
                            hardwareName);
                    }
                }

                var counters = new Dictionary<SensorType, int>();
                foreach (ISensor sensor in hardware.Sensors)
                {
                    if (!sensor.Value.HasValue) continue;

                    if (!counters.ContainsKey(sensor.SensorType))
                        counters[sensor.SensorType] = 0;

                    int index = counters[sensor.SensorType]++;
                    string sensorName = sensor.Name.Replace(":", "").Replace(";", "");
                    string value = GetFormattedValue(sensor);
                    
                    result.AppendFormat("{0}_{1}_{2}:{3}:{4}:{5};", 
                        hardwareIdentifier, 
                        sensor.SensorType.ToString().ToUpper(), 
                        index,
                        hardwareName,
                        sensorName, 
                        value);
                }
            }
            computer.Close();
        }
        catch (Exception ex)
        {
            result.Append(string.Format("LHM_ERROR:{0};", ex.Message));
        }

        try
        {
            ManagementObjectSearcher searcher = new ManagementObjectSearcher("SELECT * FROM Win32_BaseBoard");
            foreach (ManagementObject mo in searcher.Get())
            {
                string manufacturer = mo["Manufacturer"]?.ToString().Trim() ?? "N/A";
                string product = mo["Product"]?.ToString().Trim() ?? "N/A";
                result.AppendFormat("MB_INFO:{0}:{1};", manufacturer, product);
            }
        }
        catch (Exception) { /* Ignora erros de WMI */ }


        Console.Write(result.ToString());
    }
    
    private static string GetHardwareIdentifier(IHardware hardware)
    {
        switch (hardware.HardwareType)
        {
            case HardwareType.Cpu:
                return "CPU";
            case HardwareType.GpuNvidia:
            case HardwareType.GpuAmd:
            case HardwareType.GpuIntel:
                return "GPU";
            case HardwareType.Memory:
                return "RAM";
            case HardwareType.Motherboard:
                return "MB";
            case HardwareType.Storage:
                string driveType;
                 if (hardware.Sensors.Any(s => s.Name.IndexOf("Rotation", StringComparison.OrdinalIgnoreCase) >= 0) ||
                    hardware.Name.ToUpper().StartsWith("ST") || hardware.Name.ToUpper().StartsWith("WDC") ||
                    hardware.Name.ToUpper().StartsWith("TOSHIBA") || hardware.Name.ToUpper().StartsWith("HGST"))
                    driveType = "HD";
                else if (hardware.Name.ToUpper().Contains("NVME"))
                    driveType = "SSD_NVME";
                else
                    driveType = "SSD";
                return "STORAGE_" + driveType;
            default:
                return "OTHER";
        }
    }

    private static string GetFormattedValue(ISensor sensor)
    {
        float value = sensor.Value.Value;
        switch (sensor.SensorType)
        {
            case SensorType.Voltage:
                return value.ToString("F3", CultureInfo.InvariantCulture);
            case SensorType.Clock:
            case SensorType.Fan:
                return value.ToString("F0", CultureInfo.InvariantCulture);
            case SensorType.Data:
            case SensorType.SmallData:
                if (sensor.Name.IndexOf("Memory", StringComparison.OrdinalIgnoreCase) >= 0)
                    return (value * 1024).ToString("F0", CultureInfo.InvariantCulture);
                return value.ToString("F0", CultureInfo.InvariantCulture);
            case SensorType.Throughput:
                return (value / 1024 / 1024).ToString("F2", CultureInfo.InvariantCulture);
            default:
                return value.ToString("F1", CultureInfo.InvariantCulture);
        }
    }
}