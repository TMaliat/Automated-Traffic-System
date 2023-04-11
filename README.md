In our country, traffic police usually control road traffic; however, the government has policies to alleviate our country into a digital and gradually smart Bangladesh. Thus it (police hand controlled) is against the policy towards smart Bangladesh. In affinity with the previous lab assignment, we plan to develop an automated system that periodically reports the traffic status with a configurable interval. Such as, the control center can define or change the interval to five seconds; let us assume it was three seconds previously, create delay and feed traffic to the monitor center according to the recent configuration.


For this assignment,we ignore the intermediate TrafficNet and wireless mo￾dem for the communication between control center and monitor center; instead,we directly connect
Txa → Rxb 

Txb → Rxa
two UART ports of the microcontroller.In addition,we are the current UART2 to display traffic information, including traffic signal light status, on the PC display (traffic monitoring system). We are also using UART4 as control center and UART5 as monitor centert to transmit and receive traffic data or configure the traffic-signal
interval and transmission to the mechanism. Our developed system must transfer the configuration to UART5 through UART4, and receive monitor data on UART4 from UART5. The UART4 receives configuration commands from the microcontroller delivered through UART2 (currently available).Furthermore, UART4 generated a monitoring report based on data from UART5 and sent them to the PC display system through UART2.
