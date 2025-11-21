import pandas as pd
import numpy as np
import scipy.integrate as integrate
import sys
import json
import warnings

warnings.filterwarnings('ignore')

def analisar_sessao(caminho_csv):
    try:
        data = pd.read_csv(caminho_csv)
        
        if data.empty or 'Temp_CPU (C)' not in data or 'FPS' not in data:
            return {"erro": "Dados insuficientes."}

        WINDOW_SIZE = 5
        if len(data) < WINDOW_SIZE:
            data['FPS_Suavizado'] = data['FPS']
        else:
            data['FPS_Suavizado'] = data['FPS'].rolling(window=WINDOW_SIZE, center=True).mean()
            data = data.dropna()

        if data.empty:
             data = pd.read_csv(caminho_csv)
             data['FPS_Suavizado'] = data['FPS']

        tempo = data['Tempo (s)']
        temp_cpu = data['Temp_CPU (C)']
        temp_gpu = data['Temp_GPU (C)']
        fps = data['FPS_Suavizado']
        
        duracao = float(tempo.iloc[-1] - tempo.iloc[0])
        if duracao == 0: return {"erro": "Sessão curta demais."}

        # 1. INTEGRAL
        carga_cpu = integrate.simpson(y=temp_cpu, x=tempo)
        
        # 2. DERIVADA E EQUAÇÃO (Regressão Polinomial Grau 2)
        # f(x) = ax^2 + bx + c
        coefs_cpu = np.polyfit(temp_cpu, fps, 2)
        
        func_derivada_cpu = np.poly1d(coefs_cpu).deriv()
        temp_gargalo_cpu = func_derivada_cpu.roots[0]
        
        # Filtra gargalos irreais
        if coefs_cpu[0] >= -0.01 or not (40 < temp_gargalo_cpu < 110):
            temp_gargalo_cpu = 0.0

        carga_gpu = 0.0
        temp_gargalo_gpu = 0.0
        if temp_gpu.sum() > 0:
            carga_gpu = integrate.simpson(y=temp_gpu, x=tempo)
            coefs_gpu = np.polyfit(temp_gpu, fps, 2)
            pico_gpu = -coefs_gpu[1] / (2 * coefs_gpu[0])
            if coefs_gpu[0] < -0.05 and (40 < pico_gpu < 110):
                temp_gargalo_gpu = pico_gpu

        resultado = {
            "sucesso": True,
            "duracao_s": duracao,
            "fps_medio": float(fps.mean()),
            "fps_max": float(fps.max()),
            "fps_min": float(fps.min()),
            "temp_max_cpu": float(temp_cpu.max()),
            "temp_media_cpu": float(temp_cpu.mean()),
            "carga_termica_cpu": carga_cpu,
            "gargalo_cpu": temp_gargalo_cpu,
            
            # --- NOVO: Retornamos a equação encontrada ---
            "equacao_a": float(coefs_cpu[0]),
            "equacao_b": float(coefs_cpu[1]),
            "equacao_c": float(coefs_cpu[2]),
            
            "carga_termica_gpu": carga_gpu,
            "gargalo_gpu": temp_gargalo_gpu
        }
        
        for key, value in resultado.items():
            if isinstance(value, (np.integer, np.int64, np.int32)):
                resultado[key] = int(value)
            elif isinstance(value, (float, np.floating)):
                if np.isnan(value) or np.isinf(value): resultado[key] = 0.0
                else: resultado[key] = float(value)

        return resultado

    except Exception as e:
        return {"sucesso": False, "erro": str(e)}

if __name__ == "__main__":
    if len(sys.argv) > 1:
        print(json.dumps(analisar_sessao(sys.argv[1])))