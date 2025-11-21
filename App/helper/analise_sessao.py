import pandas as pd
import numpy as np
import scipy.integrate as integrate
import sys
import json
import warnings

# Silencia avisos
warnings.filterwarnings('ignore')

def analisar_sessao(caminho_csv):
    try:
        data = pd.read_csv(caminho_csv)
        
        if data.empty or 'Temp_CPU (C)' not in data or 'FPS' not in data:
            return {"erro": "Dados insuficientes no CSV."}

        # Lógica de proteção para sessões curtas
        WINDOW_SIZE = 5
        if len(data) < WINDOW_SIZE:
            data['FPS_Suavizado'] = data['FPS']
        else:
            data['FPS_Suavizado'] = data['FPS'].rolling(window=WINDOW_SIZE, center=True).mean()
            data = data.dropna()

        if data.empty:
             data = pd.read_csv(caminho_csv)
             data['FPS_Suavizado'] = data['FPS']

        if data.empty:
            return {"erro": "Nao foi possivel suavizar os dados."}

        # Cálculo da Integral (Carga Térmica)
        tempo = data['Tempo (s)']
        temp_cpu = data['Temp_CPU (C)']
        temp_gpu = data['Temp_GPU (C)']
        
        duracao_sessao = tempo.iloc[-1] - tempo.iloc[0]
        
        # Conversão forçada imediata para evitar problemas futuros
        duracao_sessao = float(duracao_sessao)

        if duracao_sessao == 0:
            return {"erro": "Sessao muito curta para analise."}

        # Integração (Regra de Simpson)
        carga_termica_cpu = integrate.simpson(y=temp_cpu, x=tempo)
        temp_media_cpu = carga_termica_cpu / duracao_sessao
        
        # Cálculo da Derivada (Gargalo)
        fps_suavizado = data['FPS_Suavizado']
        
        coefs_cpu = np.polyfit(temp_cpu, fps_suavizado, 2)
        func_derivada_cpu = np.poly1d(coefs_cpu).deriv()
        temp_gargalo_cpu = func_derivada_cpu.roots[0]
        
        if data['Temp_GPU (C)'].sum() > 0:
            coefs_gpu = np.polyfit(temp_gpu, fps_suavizado, 2)
            func_derivada_gpu = np.poly1d(coefs_gpu).deriv()
            temp_gargalo_gpu = func_derivada_gpu.roots[0]
            carga_termica_gpu = integrate.simpson(y=temp_gpu, x=tempo)
            temp_media_gpu = carga_termica_gpu / duracao_sessao
        else:
            temp_gargalo_gpu = 0.0
            carga_termica_gpu = 0.0
            temp_media_gpu = 0.0
            coefs_gpu = [0,0,0]

        resultado = {
            "sucesso": True,
            "duracao_sessao_s": duracao_sessao,
            "carga_termica_cpu_gs": carga_termica_cpu,
            "carga_termica_gpu_gs": carga_termica_gpu,
            "temp_media_cpu_c": temp_media_cpu,
            "temp_media_gpu_c": temp_media_gpu,
            "temp_gargalo_cpu_c": temp_gargalo_cpu,
            "temp_gargalo_gpu_c": temp_gargalo_gpu
        }
        
        # --- CORREÇÃO DE TIPOS (JSON CLEANING) ---
        for key, value in resultado.items():
            # Converte Inteiros do NumPy (int64, int32) para int do Python
            if isinstance(value, (np.integer, np.int64, np.int32)):
                resultado[key] = int(value)
            # Converte Floats do NumPy e trata NaN/Infinito
            elif isinstance(value, (float, np.floating)):
                if np.isnan(value) or np.isinf(value):
                    resultado[key] = 0.0
                else:
                    resultado[key] = float(value)
            # Converte Complexos
            elif isinstance(value, complex):
                resultado[key] = value.real

        return resultado

    except Exception as e:
        return {"sucesso": False, "erro": str(e)}

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(json.dumps({"sucesso": False, "erro": "Nenhum caminho de CSV fornecido."}))
        sys.exit(1)
        
    caminho_arquivo = sys.argv[1]
    resultado_final = analisar_sessao(caminho_arquivo)
    
    print(json.dumps(resultado_final))
    sys.exit(0)