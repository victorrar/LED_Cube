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

#define LED_count 5
// led = y-row >> LED_NUM & 0x01 ; y=row = array[layer][x-row] ; x-row = array[layer] ;
//double bufferization
uint8_t LED_Array_1[LED_count][LED_count];
uint8_t LED_Array_2[LED_count][LED_count];
uint8_t LED_RX[LED_count*LED_count];
uint8_t LED_Array_Rx[LED_count][LED_count]; //array for ESP8266 input LED data
uint8_t (*LED_ArrayActive)[LED_count][LED_count];
uint8_t (*LED_ArrayInactive)[LED_count][LED_count];

uint8_t USART_RxBuffer[1024];
uint8_t receivedFlag = 0;

void sendLEDData(uint8_t *layerArr);


void initCube() {
//    memset(&LED_Array_1, 0x00, LED_count * LED_count);  //TODO set to 0x00
//    memset(&LED_Array_2, 0x0B, LED_count * LED_count);

    memset(LED_RX, 0x00, LED_count*LED_count);
//    LED_RX[20] = 0xFF;

//    LED_ArrayActive = &LED_Array_1;

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
    sendLEDData(LED_RX + (layer * LED_count));

    // enable next pin
//    HAL_GPIO_WritePin(LED_LAYER_1_GPIO_Port, (LED_LAYER_1_Pin >> layer), GPIO_PIN_SET);
    // reset prev pin
//    HAL_GPIO_WritePin(LED_LAYER_1_GPIO_Port, (LED_LAYER_1_Pin >> ((layer - 1) % LED_count)),
//            GPIO_PIN_RESET);

    switch (layer) {
        case 1:
            HAL_GPIO_WritePin(LED_LAYER_1_GPIO_Port, LED_LAYER_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_LAYER_5_GPIO_Port, LED_LAYER_5_Pin, GPIO_PIN_RESET);
            break;
        case 2:
            HAL_GPIO_WritePin(LED_LAYER_2_GPIO_Port, LED_LAYER_2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_LAYER_1_GPIO_Port, LED_LAYER_1_Pin, GPIO_PIN_RESET);
            break;
        case 3:
            HAL_GPIO_WritePin(LED_LAYER_3_GPIO_Port, LED_LAYER_3_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_LAYER_2_GPIO_Port, LED_LAYER_2_Pin, GPIO_PIN_RESET);
            break;
        case 4:
            HAL_GPIO_WritePin(LED_LAYER_4_GPIO_Port, LED_LAYER_4_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_LAYER_3_GPIO_Port, LED_LAYER_3_Pin, GPIO_PIN_RESET);
            break;
        case 0:
            HAL_GPIO_WritePin(LED_LAYER_5_GPIO_Port, LED_LAYER_5_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_LAYER_4_GPIO_Port, LED_LAYER_4_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }

    layer++;
    layer %= LED_count;
}

void sendLEDData(uint8_t *layerArr) {
//    HAL_DMA_Start_IT(&hdma_spi1_tx, (uint32_t) (layerArr), SPI1_BASE, LED_count);
    HAL_SPI_Transmit_IT(&hspi1, layerArr, LED_count);
}


void processUsartCommand() {
    if (receivedFlag == 1) {
//        swapDisplayArray();
        receivedFlag = 0;
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == &hspi1) {
        HAL_GPIO_WritePin(Latch_GPIO_Port, Latch_Pin, GPIO_PIN_SET);
//        HAL_Delay(1);
        int i = 1;
        while (--i){
            __NOP();
        }
        HAL_GPIO_WritePin(Latch_GPIO_Port, Latch_Pin, GPIO_PIN_RESET);
//        displayLayerFlag = 1;
        displayLayer();
    }
}

void tmp() {
    uint8_t commandId = 0x00;
    HAL_UART_Receive_IT(&huart1, &commandId, 1);  //TODO receive command ID

    switch (commandId) {
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

    if (huart->RxXferSize == 1) {    //received command
        uint8_t command = *(huart->pRxBuffPtr - huart->RxXferSize);
        switch (command) {
            case 0x01:  //receive led array command
            {
//                HAL_StatusTypeDef tst = HAL_UART_Receive_IT(&huart1, &((*LED_ArrayActive)[layer][0]),
//                                                            LED_count * LED_count);
                HAL_UART_Receive_IT(&huart1, LED_RX, LED_count * LED_count);
//                memcpy((*LED_ArrayActive)[0], LED_RX, LED_count);
//                memcpy((*LED_ArrayActive)[1], (LED_RX+LED_count), LED_count);
//                memcpy((*LED_ArrayActive)[2], (LED_RX+2*LED_count), LED_count);
//                memcpy((*LED_ArrayActive)[3], (LED_RX+3*LED_count), LED_count);
//                memcpy((*LED_ArrayActive)[4], (LED_RX+4*LED_count), LED_count);


//                if(tst != HAL_OK){
//                    while(1){
//                        __NOP();
//                    }
//                }
            }    //TODO chech if it runs DMA callback
                break;
            case 0x02:  //display layer test command
                displayLayer();
                break;
            default:    //some strange shit occurred
                while (0) {
                    __NOP();
                }
                break;
        }

    }
    if (huart->RxXferSize > 1) { //Received LED array
        receivedFlag = 1;
        processUsartCommand();
        HAL_UART_Receive_IT(huart, USART_RxBuffer, 1);
    }
//    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
}