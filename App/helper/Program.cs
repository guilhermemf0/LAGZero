using System;
using System.Globalization;
using System.Text;
using LibreHardwareMonitor.Hardware;

// ATENÇÃO: ESTE CÓDIGO RESTAURA A VERSÃO ORIGINAL DO Program.cs

namespace TempReader
{
    // Visitor para atualização dos sensores
    public class UpdateVisitor : IVisitor
    {
        public void VisitComputer(IComputer computer)
        {
            computer.Traverse(this);
        }
        public void VisitHardware(IHardware hardware)
        {
            hardware.Update();
            foreach (IHardware subHardware in hardware.SubHardware) subHardware.Accept(this);
        }
        public void VisitSensor(ISensor sensor) { }
        public void VisitParameter(IParameter parameter) { }
    }

    class Program
    {
        static void Main(string[] args)
        {
            // Evita problemas de formatação de número (ponto vs vírgula)
            CultureInfo.DefaultThreadCurrentCulture = CultureInfo.InvariantCulture;
            CultureInfo.DefaultThreadCurrentUICulture = CultureInfo.InvariantCulture;

            var computer = new Computer
            {
                IsCpuEnabled = true,
                IsGpuEnabled = true,
                IsMemoryEnabled = true,
                IsMotherboardEnabled = true,
                IsStorageEnabled = true
            };

            try
            {
                computer.Open();
                computer.Accept(new UpdateVisitor());

                StringBuilder output = new StringBuilder();

                foreach (var hardware in computer.Hardware)
                {
                    string hardwareKeyPrefix = "";
                    switch (hardware.HardwareType)
                    {
                        case HardwareType.Cpu: hardwareKeyPrefix = "CPU"; break;
                        case HardwareType.GpuNvidia:
                        case HardwareType.GpuAmd:
                        case HardwareType.GpuIntel: hardwareKeyPrefix = "GPU"; break;
                        case HardwareType.Motherboard: hardwareKeyPrefix = "MB"; break;
                        case HardwareType.Storage: hardwareKeyPrefix = "STORAGE"; break;
                        case HardwareType.Memory: hardwareKeyPrefix = "RAM"; break;
                    }

                    if (string.IsNullOrEmpty(hardwareKeyPrefix)) continue;

                    int tempIndex = 0, loadIndex = 0, dataIndex = 0, powerIndex = 0;

                    foreach (var sensor in hardware.Sensors)
                    {
                        string sensorKey = "";
                        switch (sensor.SensorType)
                        {
                            case SensorType.Temperature: sensorKey = $"{hardwareKeyPrefix}_TEMPERATURE_{tempIndex++}"; break;
                            case SensorType.Load: sensorKey = $"{hardwareKeyPrefix}_LOAD_{loadIndex++}"; break;
                            case SensorType.Data: sensorKey = $"{hardwareKeyPrefix}_DATA_{dataIndex++}"; break;
                            case SensorType.Power: sensorKey = $"{hardwareKeyPrefix}_POWER_{powerIndex++}"; break;
                        }

                        if (!string.IsNullOrEmpty(sensorKey) && sensor.Value.HasValue)
                        {
                            string sensorName = string.IsNullOrEmpty(sensor.Name) ? "No Sensor" : sensor.Name;
                            output.Append($"{sensorKey}:{hardware.Name}:{sensorName}:{sensor.Value.GetValueOrDefault().ToString(CultureInfo.InvariantCulture)};");
                        }
                    }
                }
                Console.Write(output.ToString());
            }
            catch (Exception)
            {
                // Ignora exceções para garantir que o programa não trave
            }
            finally
            {
                computer.Close();
            }
        }
    }
}