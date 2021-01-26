#ifndef CRITICAL_H
#define CRITICAL_H

#define ENTER_CRITICAL() int xxx_prevState = __disable_irq();{
#define EXIT_CRITICAL() if(!xxx_prevState){__enable_irq();}}    

#endif