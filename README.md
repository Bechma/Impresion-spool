# impresion-spool

![Alt text](https://github.com/Bechma/impresion-spool/blob/master/spool.png)

To make it run I recommend the following steps:

- make
- ./servidor 123 &
- ./cliente 123 5
- Behold the communication of process through FIFO

I should mention the first argument you give to "cliente" must be the same that you gave before to servidor(the order matters) and the second argument of client is the number of times the process will send heaps of information thus it is not mandatory to put 5, you can change it.


Did in "Operating Systems" UGR
