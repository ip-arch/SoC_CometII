/* Commet class body */ 
#ifndef COMET2_H
#define COMET2_H
#endif
declare    comet2  {   
    input     datai[16]; 
    output    datao[16]; 
    output    adrs[16];   
    func_out  memory_read(adrs):datai; 
    func_out  memory_write(adrs,datao); 
    func_out  hlt;
    func_in   start();
    func_in   ext_int();
} 

