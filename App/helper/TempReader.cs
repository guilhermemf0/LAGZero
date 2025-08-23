using System;
using System.Threading; // Adicionado para usar o Thread.Sleep
using LibreHardwareMonitor.Hardware;

// Classe para atualizar os dados do hardware
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

public class HardwareMonitor
{
    public static void Main()
    {
        // Cria uma instância do computador para monitorar
        Computer computer = new Computer
        {
            IsCpuEnabled = true, // Habilita o monitoramento da CPU
        };

        try
        {
            computer.Open();
            Thread.Sleep(500); // Adiciona um atraso de 500ms para a inicialização do hardware
            computer.Accept(new UpdateVisitor());

            // Encontra o hardware da CPU usando um loop
            foreach (IHardware hardware in computer.Hardware)
            {
                if (hardware.HardwareType == HardwareType.Cpu)
                {
                    // Encontra o PRIMEIRO sensor de temperatura
                    foreach (ISensor sensor in hardware.Sensors)
                    {
                        if (sensor.SensorType == SensorType.Temperature)
                        {
                            if (sensor.Value.HasValue)
                            {
                                // Imprime o valor da temperatura no console
                                Console.Write(sensor.Value.Value);
                                computer.Close();
                                return;
                            }
                        }
                    }
                }
            }
        }
        catch (Exception)
        {
            // Em caso de erro, não imprime nada
        }
        finally
        {
            computer.Close();
        }
    }
}
