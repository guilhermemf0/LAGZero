using System;
using System.Threading;
using System.Globalization;
using LibreHardwareMonitor.Hardware;

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
        };

        string cpuTemp = "-1";
        string motherboardTemp = "-1";
        string gpuTemp = "-1";

        try
        {
            computer.Open();
            Thread.Sleep(500);
            computer.Accept(new UpdateVisitor());

            foreach (IHardware hardware in computer.Hardware)
            {
                // Encontra a temperatura da CPU
                if (hardware.HardwareType == HardwareType.Cpu)
                {
                    foreach (ISensor sensor in hardware.Sensors)
                    {
                        if (sensor.SensorType == SensorType.Temperature && sensor.Value.HasValue)
                        {
                            cpuTemp = sensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture);
                            break;
                        }
                    }
                }
                // Encontra a temperatura da Placa-mãe
                if (hardware.HardwareType == HardwareType.Motherboard)
                {
                    ISensor systemSensor = null;
                    ISensor fallbackSensor = null;

                    // Procura nos sub-componentes
                    foreach (IHardware subHardware in hardware.SubHardware)
                    {
                        subHardware.Update();
                        foreach (ISensor sensor in subHardware.Sensors)
                        {
                            if (sensor.SensorType == SensorType.Temperature && sensor.Value.HasValue)
                            {
                                // Tenta encontrar o sensor "System" preferencialmente
                                if (sensor.Name.Contains("System"))
                                {
                                    systemSensor = sensor;
                                    break; // Encontrou o sensor preferencial
                                }
                                // Guarda o primeiro sensor genérico como alternativa
                                if (fallbackSensor == null)
                                {
                                    fallbackSensor = sensor;
                                }
                            }
                        }
                        if (systemSensor != null) break;
                    }

                    // Usa o sensor "System" se encontrou, senão usa a alternativa
                    ISensor finalSensor = systemSensor != null ? systemSensor : fallbackSensor;
                    if (finalSensor != null)
                    {
                        motherboardTemp = finalSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture);
                    }
                }
                // Encontra a temperatura da GPU
                if (hardware.HardwareType == HardwareType.GpuNvidia || hardware.HardwareType == HardwareType.GpuAmd)
                {
                    ISensor coreSensor = null;
                    foreach (ISensor sensor in hardware.Sensors)
                    {
                        if (sensor.SensorType == SensorType.Temperature && sensor.Name.Contains("Core") && sensor.Value.HasValue)
                        {
                            coreSensor = sensor;
                            break;
                        }
                    }
                    if (coreSensor != null)
                    {
                        gpuTemp = coreSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture);
                    }
                }
            }
        }
        catch (Exception) { }
        finally
        {
            computer.Close();
        }

        Console.Write(String.Format("CPU:{0};MOTHERBOARD:{1};GPU:{2};", cpuTemp, motherboardTemp, gpuTemp));
    }
}
