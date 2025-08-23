using System;
using System.Threading;
using System.Globalization;
using LibreHardwareMonitor.Hardware;
using System.IO; // Necessário para escrever no ficheiro de log

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
        // O ficheiro de log será criado ao lado do TempReader.exe
        string logPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "log.txt");

        try
        {
            File.WriteAllText(logPath, "Helper iniciado em: " + DateTime.Now + "\n");
            File.AppendAllText(logPath, "Diretório de Trabalho: " + Directory.GetCurrentDirectory() + "\n");

            Computer computer = new Computer
            {
                IsCpuEnabled = true,
                IsGpuEnabled = true,
                IsMotherboardEnabled = true,
            };

            File.AppendAllText(logPath, "Objeto 'Computer' criado.\n");
            computer.Open();
            File.AppendAllText(logPath, "Comando 'computer.Open()' executado.\n");

            Thread.Sleep(1500);
            computer.Accept(new UpdateVisitor());
            File.AppendAllText(logPath, "Visitor aceite, sensores atualizados.\n");

            string cpuTemp = "-1", motherboardTemp = "-1", gpuTemp = "-1";

            foreach (IHardware hardware in computer.Hardware)
            {
                hardware.Update();
                if (hardware.HardwareType == HardwareType.Cpu)
                {
                    ISensor packageSensor = null, coreMaxSensor = null, fallbackSensor = null;
                    Action<IHardware> findCpuSensors = (hw) => {
                        foreach (ISensor sensor in hw.Sensors)
                            if (sensor.SensorType == SensorType.Temperature && sensor.Value.HasValue) {
                                if (sensor.Name.IndexOf("Package", StringComparison.OrdinalIgnoreCase) >= 0) { packageSensor = sensor; return; }
                                if (coreMaxSensor == null && sensor.Name.IndexOf("Core Max", StringComparison.OrdinalIgnoreCase) >= 0) { coreMaxSensor = sensor; }
                                if (fallbackSensor == null) { fallbackSensor = sensor; }
                            }
                    };
                    findCpuSensors(hardware);
                    if (packageSensor == null) foreach (IHardware subHardware in hardware.SubHardware) { subHardware.Update(); findCpuSensors(subHardware); if (packageSensor != null) break; }
                    ISensor finalSensor = packageSensor ?? coreMaxSensor ?? fallbackSensor;
                    if (finalSensor != null) cpuTemp = finalSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture);
                }
                if (hardware.HardwareType == HardwareType.Motherboard)
                {
                    ISensor systemSensor = null, fallbackSensor = null;
                    foreach (IHardware subHardware in hardware.SubHardware) {
                        subHardware.Update();
                        foreach (ISensor sensor in subHardware.Sensors)
                            if (sensor.SensorType == SensorType.Temperature && sensor.Value.HasValue) {
                                if (sensor.Name.IndexOf("System", StringComparison.OrdinalIgnoreCase) >= 0) { systemSensor = sensor; break; }
                                if (fallbackSensor == null) { fallbackSensor = sensor; }
                            }
                        if (systemSensor != null) break;
                    }
                    ISensor finalSensor = systemSensor ?? fallbackSensor;
                    if (finalSensor != null) motherboardTemp = finalSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture);
                }
                if (hardware.HardwareType == HardwareType.GpuNvidia || hardware.HardwareType == HardwareType.GpuAmd)
                {
                    ISensor hotspotSensor = null, coreSensor = null, fallbackSensor = null;
                    foreach (ISensor sensor in hardware.Sensors)
                        if (sensor.SensorType == SensorType.Temperature && sensor.Value.HasValue) {
                            if (sensor.Name.IndexOf("Hotspot", StringComparison.OrdinalIgnoreCase) >= 0) { hotspotSensor = sensor; break; }
                            if (coreSensor == null && sensor.Name.IndexOf("Core", StringComparison.OrdinalIgnoreCase) >= 0) { coreSensor = sensor; }
                            if (fallbackSensor == null) { fallbackSensor = sensor; }
                        }
                    ISensor finalSensor = hotspotSensor ?? coreSensor ?? fallbackSensor;
                    if (finalSensor != null) gpuTemp = finalSensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture);
                }
            }

            File.AppendAllText(logPath, string.Format("Temperaturas encontradas - CPU: {0}, Placa-mãe: {1}, GPU: {2}\n", cpuTemp, motherboardTemp, gpuTemp));
            Console.Write(String.Format("CPU:{0};MOTHERBOARD:{1};GPU:{2};", cpuTemp, motherboardTemp, gpuTemp));
            File.AppendAllText(logPath, "Valores impressos na consola com sucesso.\n");
        }
        catch (Exception ex)
        {
            File.AppendAllText(logPath, "\n\n--- OCORREU UMA EXCEÇÃO ---\n" + ex.ToString());
        }
    }
}
