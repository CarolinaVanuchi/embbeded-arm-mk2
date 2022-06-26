#ifndef _WAVE_GEN_ 
#define _WAVE_GEN_

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "boolarray.h"

/**
 * Definir pi
 */
#define PI (acos(-1.0))

/**
 * @param step: passo atual a calcular
 * @param step_max: numero total de passos
 * @param fmin: frequência de início de acelereção. Valor menor
 * @param fmin: frequência de aceleracao máxima. Valor maior
 * @return retorna em decimal a frêquencia para o pulso 'step' conforme uma curva de aceleração senoidal
 */
double accCurveSin(uint32_t step, uint32_t step_max, uint32_t fmin, uint32_t fmax){
    if(fmax <= fmin) fmax = fmin + 1;

    // valor min é igual a fmin para step = 0 ou step = step_max, valor max ocorre no meio da senoide, onde será seno(pi/2)*fmax
    return (fmax-fmin)*sin(((double)step/(step_max - 1)*PI)) + fmin;
}

/**
 * @brief array que define uma forma de onda digital a partir de pontos
 */
typedef struct{
    boolArray_t *points;                                                            /**< array de booleanas que representa os pontos, possui dentro um ponteiro bool* e tamanho size_t */
    size_t index;                                                                   /**< estado interno de leitura. Numero atual de leitura dos valores, usado na função waveRead(), pode ser resetado por waveReadReset() */
    double period;                                                                  /**< periodo de tempo para cada ponto, dado em segundos */
}wave_t;

/**
 * @param step_max: numero total de passos
 * @param fmin: frequência de início de acelereção. Valor menor
 * @param fmin: frequência de aceleracao máxima. Valor maior
 * @param res: resolução de pontos conforme período da maior frequência. A maior frequencia possui menor período, assim o período da onda toda será o período de 'fmax' devidio por 'res'
 * @return retorna uma 'wave_t' que pode ser lida com waveRead(). Deve ser desalocada com waveDelete()
 */
wave_t *waveGenStepMotorSineAcceleration(uint32_t step_max, uint32_t fmin, uint32_t fmax, uint32_t res){
    // menor período
    // ESP_LOGI("RY", "%i", step_max);

    double t = 1.0/fmax/res;
    // ESP_LOGI("RY", "%f", t);
    wave_t *wave = calloc(1, sizeof(*wave));
    wave->points = boolArrayCreate();
    wave->index = 0;
    wave->period = t;

    // para cada passo
    for(size_t step = 0; step < step_max; step++){
        // numero de pontos por passo
        uint32_t pps = ceil(1.0/(accCurveSin(step, step_max, fmin, fmax)*t));
        // ESP_LOGI("AAA", "%i", pps);

        // para cada ponto
        for(size_t point = 0; point < pps; point++){
            // adicionar um ponto no array
            // ESP_LOGI("AAA", "%d",  (point < pps/2 ? false : true));
            boolArrayAdd(wave->points, (point < pps/2 ? false : true));
        }   
    }

    return wave;
}

/**
 * @param wave: ponteiro instância de 'wave_t' 
 * @return retorna o próximo valor de um ponto de wave. Se acabarem os pontos sempre retornará false/zero. Para resetar deve se chamar waveReadReset()  
 */
bool waveRead(wave_t *wave){
    bool value = wave->points->array[wave->index];
    wave->index++;
    
    if(wave->index > wave->points->size){
        wave->index--;
        return false;
    }
    else{
        return value;
    }
}

/**
 * @brief reseta contador interno de leitura
 * @param wave: ponteiro instância de 'wave_t' 
 */
void waveReadReset(wave_t *wave){
    wave->index = 0;
}

/**
 * @brief printa na tela em formato csv os pontos da wave. Pode ser chamado no programa que pode então ter sua saída redirecionada para um arquivo. Ex: "$ ./main.exe > out.csv"
 * @param wave: ponteiro instância de 'wave_t' 
 */
void wavePrint(wave_t *wave){
    for(size_t i = 0; i < wave->points->size; i++){
        printf("%d,%c\n", i, (wave->points->array[i] == true ? '1' : '0'));
    }
}

/**
 * @brief desaloca memória da instância 'wave_t'
 * @param wave: ponteiro instância de 'wave_t' 
 */
void waveDelete(wave_t *wave){
    boolArrayDelete(wave->points);
    free(wave);
}

#endif