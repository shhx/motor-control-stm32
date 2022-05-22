# motor_control_stm32
  Motor control with independent frequency, duty cycle and delay for each one. Up to 6 motors.
  
  
  Timers chained to start in sync:
  
```
            20ns          20ns
  TIM1(PE9) ->  TIM2(PA15) -> TIM5(PA0)
                TIM3(PC6)     TIM9(PA2)
                TIM4(PD12)
```