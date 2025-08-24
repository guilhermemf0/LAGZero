using System;
using System.Threading;
using System.Globalization;
using LibreHardwareMonitor.Hardware;
using System.Text;

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
            IsStorageEnabled = true
        };

        StringBuilder result = new StringBuilder();

        try
        {
            computer.Open();
            Thread.Sleep(1000);
            computer.Accept(new UpdateVisitor());

            foreach (IHardware hardware in computer.Hardware)
            {
                hardware.Update();
                string hardwareName = hardware.Name.Replace(":", "").Replace(";", "");

                if (hardware.HardwareType == HardwareType.Cpu)
                {
                    ISensor tempSensor = FindBestSensor(hardware, new[] { "Package", "Core (Tctl/Tdie)" });
                    if (tempSensor != null && tempSensor.Value.HasValue)
                    {
                        result.AppendFormat("CPU:{0}:{1};", hardwareName, tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                    }
                }

                if (hardware.HardwareType == HardwareType.GpuAmd || hardware.HardwareType == HardwareType.GpuNvidia)
                {
                    ISensor tempSensor = FindBestSensor(hardware, new[] { "Hotspot", "Core" });
                    if (tempSensor != null && tempSensor.Value.HasValue)
                    {
                        result.AppendFormat("GPU:{0}:{1};", hardwareName, tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                    }
                }

                if (hardware.HardwareType == HardwareType.Motherboard)
                {
                    ISensor tempSensor = FindBestSensor(hardware, new[] { "System", "Chipset", "CPU Socket", "VRM", "Mainboard" });
                    if (tempSensor == null)
                    {
                        foreach (IHardware subHardware in hardware.SubHardware)
                        {
                            subHardware.Update();
                            tempSensor = FindBestSensor(subHardware, new[] { "System", "Chipset", "CPU Socket", "VRM", "Mainboard" });
                            if (tempSensor != null) break;
                        }
                    }
                    if (tempSensor != null && tempSensor.Value.HasValue)
                    {
                        result.AppendFormat("MOTHERBOARD:{0}:{1};", hardwareName, tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                    }
                }

                if (hardware.HardwareType == HardwareType.Storage)
                {
                    ISensor tempSensor = FindBestSensor(hardware, new[] { "Temperature" });
                    if (tempSensor != null && tempSensor.Value.HasValue)
                    {
                        // --- CORREÇÃO FINAL: Detecta o tipo de drive pelo NOME do sensor ---
                        string driveType;
                        bool hasRotationSensor = false;

                        // Procura por um sensor cujo NOME contenha "Rotation", que é mais compatível
                        foreach (ISensor sensor in hardware.Sensors)
                        {
                            if (sensor.Name.IndexOf("Rotation", StringComparison.OrdinalIgnoreCase) >= 0)
                            {
                                hasRotationSensor = true;
                                break;
                            }
                        }

                        if (hasRotationSensor)
                        {
                            driveType = "HD";
                        }
                        else
                        {
                            string nameUpper = hardware.Name.ToUpper();
                            if (nameUpper.Contains("NVME") || nameUpper.Contains("M.2"))
                            {
                                driveType = "SSD M.2";
                            }
                            else
                            {
                                driveType = "SSD";
                            }
                        }

                        result.AppendFormat("STORAGE_{0}:{1}:{2}:{3};",
                            hardwareName.Replace(" ", "_"),
                            hardwareName,
                            driveType,
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
