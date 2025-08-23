using System;
using System.Threading;
using System.Globalization;
using LibreHardwareMonitor.Hardware;
using System.Text; // Usaremos o StringBuilder para mais eficiência

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
            IsStorageEnabled = true // Habilita o monitoramento de discos
        };

        // Usaremos um StringBuilder para construir a string de saída
        StringBuilder result = new StringBuilder();

        try
        {
            computer.Open();
            Thread.Sleep(1000);
            computer.Accept(new UpdateVisitor());

            int storageIndex = 0; // Contador para os discos

            foreach (IHardware hardware in computer.Hardware)
            {
                hardware.Update();

                // CPU
                if (hardware.HardwareType == HardwareType.Cpu)
                {
                    ISensor tempSensor = FindBestSensor(hardware, new string[] { "Package", "Core Max" });
                    if (tempSensor != null)
                        result.AppendFormat("CPU:{0};", tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                }
                // Placa-mãe
                if (hardware.HardwareType == HardwareType.Motherboard)
                {
                    foreach (IHardware subHardware in hardware.SubHardware)
                    {
                        subHardware.Update();
                        ISensor tempSensor = FindBestSensor(subHardware, new string[] { "System" });
                        if (tempSensor != null) {
                            result.AppendFormat("MOTHERBOARD:{0};", tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                            break;
                        }
                    }
                }
                // GPU
                if (hardware.HardwareType == HardwareType.GpuNvidia || hardware.HardwareType == HardwareType.GpuAmd)
                {
                    ISensor tempSensor = FindBestSensor(hardware, new string[] { "Hotspot", "Core" });
                    if (tempSensor != null)
                        result.AppendFormat("GPU:{0};", tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                }
                // Discos de Armazenamento (HDD/SSD)
                if (hardware.HardwareType == HardwareType.Storage)
                {
                    ISensor tempSensor = FindBestSensor(hardware, new string[] { "Temperature" });
                    if (tempSensor != null) {
                        // Adiciona o nome do disco para melhor identificação
                        string driveName = hardware.Name.Replace(" ", "_");
                        result.AppendFormat("STORAGE_{0}_{1}:{2};", storageIndex, driveName, tempSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture));
                        storageIndex++;
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

    // Função auxiliar para encontrar o melhor sensor com base numa lista de prioridades
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
                        return sensor; // Encontrou o sensor prioritário
                    if (fallbackSensor == null)
                        fallbackSensor = sensor; // Guarda o primeiro como alternativa
                }
            }
        }
        return fallbackSensor; // Retorna a alternativa se nenhum prioritário for encontrado
    }
}
