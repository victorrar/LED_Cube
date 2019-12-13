#include <stdbool.h>
#include "string.h"
#include "ledController.h"

#define SET_FLAG(flag, some_test, some_flag) ((some_test) ? ((flag) |= some_flag) : ((flag) &= ~some_flag))
//
// Created by Victor on 05.12.2019.
//
extern DMA_HandleTypeDef hdma_spi1_tx;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

#define LED_count 8
// led = y-row >> LED_NUM & 0x01 ; y=row = array[layer][x-row] ; x-row = array[layer] ;
//double bufferization
uint8_t LED_Array_1[LED_count][LED_count];
uint8_t LED_Array_2[LED_count][LED_count];
uint8_t LED_Array_Rx[LED_count][LED_count]; //array for ESP8266 input LED data
uint8_t (*LED_ArrayActive)[LED_count][LED_count];
uint8_t (*LED_ArrayInactive)[LED_count][LED_count];

uint8_t USART_RxBuffer[1024];
uint8_t receivedFlag = 0;

void sendLEDData(uint8_t *layerArr);


void initCube() {
    memset(&LED_Array_1, 0x00, LED_count * LED_count);
    memset(&LED_Array_2, 0x00, LED_count * LED_count);

    LED_ArrayActive = &LED_Array_1;

    HAL_SPI_TxCpltCallback(&hspi1);
}

void setLED(int X, int Y, int Z, bool state) {
    SET_FLAG((*LED_ArrayInactive)[Z][Y], state, 0x01 << X);
}

void copyActiveArray() {
    memcpy(LED_ArrayActive, LED_ArrayInactive, LED_count * LED_count);
}

void swapDisplayArray() {
    if (LED_ArrayActive == &LED_Array_1) {
        LED_ArrayActive = &LED_Array_2;
        LED_ArrayInactive = &LED_Array_1;
    } else {
        LED_ArrayActive = &LED_Array_1;
        LED_ArrayInactive = &LED_Array_2;
    }
}

void setDisplayArray(int arr) {

}

uint8_t layer = 0;
//uint8_t displayLayerFlag = 0;

void displayLayer() {
    sendLEDData(*LED_ArrayActive[layer]);
    switch(layer){
        case 0:
            break;
//            HAL_GPIO_WritePin()
    }
    layer++;
    layer %= LED_count;
}

void sendLEDData(uint8_t *layerArr) {
    HAL_DMA_Start_IT(&hdma_spi1_tx, (uint32_t) (layerArr), SPI1_BASE, LED_count);
}


void processUsartCommand() {
    if(receivedFlag == 1) {
        swapDisplayArray();
        receivedFlag = 0;
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == &hspi1) {
        HAL_GPIO_WritePin(Latch_GPIO_Port, Latch_Pin, GPIO_PIN_SET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(Latch_GPIO_Port, Latch_Pin, GPIO_PIN_RESET);
//        displayLayerFlag = 1;
        displayLayer();
    }
}

void tmp(){
    uint8_t commandId = 0x00;
    HAL_UART_Receive_IT(&huart1, &commandId, 1);  //TODO receive command ID

    switch (commandId){
        case 0x00:
            //error
            break;
        case 0x01:
            //data

            break;
    }

    //receive rest of command
    //after idle detection abort reception process command
    //after transfer complete process command

}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if(huart->RxXferCount == 1) {
        HAL_UART_Receive_DMA(&huart1, (uint8_t *) LED_ArrayInactive, LED_count * LED_count);
    }
    if(huart->RxXferCount > 1) {
        receivedFlag = 1;
        processUsartCommand();
    }
}