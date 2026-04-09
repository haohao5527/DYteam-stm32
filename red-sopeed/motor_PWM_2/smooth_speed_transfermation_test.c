uint8_t speed = 50;
uint8_t a = 0;
void Motor_speed(uint8_t speed)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, speed);
}
int forward(uint8_t speed)
{
    uint8_t j = 0;
    while(j != speed)
    {
        if(a==0)
        {
            for (uint8_t i = 0; i < speed; i++)
            {
                j = i;
                Motor_speed(i);
                HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
                HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
            }
            a=1;
            return a;
        }
        else if(a==2)
        {
            for (uint8_t i = speed; i > 0; i--)
            {
                j = i;
                Motor_speed(i);
            }
            a = 0;
            continue;
        }

    }
}

int backward(uint8_t speed)
{
    uint8_t j = 0;
    while(j != speed)
    {
        if(a==0)
        {
            for (uint8_t i = 0; i < speed; i++)
            {
                j = i;
                Motor_speed(i);
                HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
            }
            a=2;
            return a;
        }
        else if(a==1)
        {
            for (uint8_t i = speed; i > 0; i--)
            {
                j = i;
                Motor_speed(i);
            }
            a = 0;
            continue;
        }

    }
}