using System;
using System.Threading;
using System.Globalization;
using LibreHardwareMonitor.Hardware;
using System.Text;
using System.Linq; // Adicionado para facilitar a busca por sensores

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
        Computer computer = new Computer
        {
            IsCpuEnabled = true,
            IsGpuEnabled = true,
            IsMotherboardEnabled = true,
            IsStorageEnabled = true,
            IsMemoryEnabled = true // NOVO: Habilita o monitoramento de memória
        };

        StringBuilder result = new StringBuilder();

        try
        {
            computer.Open();
            Thread.Sleep(1000); // Dê um tempo para os sensores inicializarem
            computer.Accept(new UpdateVisitor());

            foreach (IHardware hardware in computer.Hardware)
            {
                hardware.Update();
                string hardwareName = hardware.Name.Replace(":", "").Replace(";", "");

                if (hardware.HardwareType == HardwareType.Cpu)
                {
                    // Sensor de temperatura do pacote (geral)
                    ISensor tempSensor = FindBestSensor(hardware, new[] { "Package", "Core (Tctl/Tdie)" });
                    if (tempSensor != null && tempSensor.Value.HasValue)
                    {
                        result.AppendFormat("CPU:{0}:{1};", hardwareName, tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                    }

                    // NOVO: Sensor de uso total da CPU
                    ISensor usageSensor = hardware.Sensors.FirstOrDefault(s => s.SensorType == SensorType.Load && s.Name == "CPU Total");
                    if (usageSensor != null && usageSensor.Value.HasValue)
                    {
                        result.AppendFormat("CPU_USAGE:{0}:{1};", "Total", usageSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                    }
                    
                    // NOVO: Sensores de temperatura por núcleo
                    foreach (ISensor sensor in hardware.Sensors)
                    {
                        if (sensor.SensorType == SensorType.Temperature && sensor.Name.Contains("Core #"))
                        {
                            string coreId = new string(sensor.Name.Where(char.IsDigit).ToArray());
                            if (!string.IsNullOrEmpty(coreId) && sensor.Value.HasValue)
                            {
                                result.AppendFormat("CPU_CORE_{0}:{1}:{2};", coreId, "Core " + coreId, sensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                            }
                        }
                    }
                }

                if (hardware.HardwareType == HardwareType.GpuAmd || hardware.HardwareType == HardwareType.GpuNvidia)
                {
                    // Temperatura da GPU
                    ISensor tempSensor = FindBestSensor(hardware, new[] { "Hotspot", "Core" });
                    if (tempSensor != null && tempSensor.Value.HasValue)
                    {
                        result.AppendFormat("GPU:{0}:{1};", hardwareName, tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                    }
                    
                    // NOVO: Uso da GPU
                    ISensor gpuUsageSensor = hardware.Sensors.FirstOrDefault(s => s.SensorType == SensorType.Load && (s.Name == "GPU Core" || s.Name == "D3D 3D"));
                     if (gpuUsageSensor != null && gpuUsageSensor.Value.HasValue)
                    {
                        result.AppendFormat("GPU_USAGE:{0}:{1};", "Core", gpuUsageSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                    }
                }
                
                // NOVO: Monitoramento de Memória RAM
                if (hardware.HardwareType == HardwareType.Memory)
                {
                    ISensor memUsedSensor = hardware.Sensors.FirstOrDefault(s => s.SensorType == SensorType.Data && s.Name == "Memory Used");
                    if (memUsedSensor != null && memUsedSensor.Value.HasValue)
                    {
                        // O valor é em GB, convertemos para MB
                        result.AppendFormat("RAM_USAGE:{0}:{1};", "Used", (memUsedSensor.Value.Value * 1024).ToString("F0", CultureInfo.InvariantCulture));
                    }
                }

                if (hardware.HardwareType == HardwareType.Storage)
                {
                    ISensor tempSensor = FindBestSensor(hardware, new[] { "Temperature" });
                    if (tempSensor != null && tempSensor.Value.HasValue)
                    {
                        string driveType;
                        bool hasRotationSensor = hardware.Sensors.Any(s => s.Name.IndexOf("Rotation", StringComparison.OrdinalIgnoreCase) >= 0);

                        if (hasRotationSensor) driveType = "HD";
                        else {
                            string nameUpper = hardware.Name.ToUpper();
                            if (nameUpper.Contains("NVME") || nameUpper.Contains("M.2")) driveType = "SSD M.2";
                            else driveType = "SSD";
                        }

                        result.AppendFormat("STORAGE_{0}:{1}:{2}:{3};",
                            hardwareName.Replace(" ", "_"), hardwareName, driveType,
                            tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                    }
                }
            }
        }
        catch (Exception) { }
        finally
        {
            computer.Close();
        }

        Console.Write(result.ToString());
    }

    private static ISensor FindBestSensor(IHardware hardware, string[] priorityNames)
    {
        ISensor fallbackSensor = null;
        foreach (string name in priorityNames)
        {
            foreach (ISensor sensor in hardware.Sensors)
            {
                if (sensor.SensorType == SensorType.Temperature && sensor.Value.HasValue)
                {
                    if (sensor.Name.IndexOf(name, StringComparison.OrdinalIgnoreCase) >= 0)
                        return sensor;
                    if (fallbackSensor == null)
                        fallbackSensor = sensor;
                }
            }
        }
        return fallbackSensor;
    }
}
