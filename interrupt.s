    .globl default_exception_handler
    .globl default_interrupt_handler
    .globl exception_handlers


default_exception_handler:
    jump $

default_interrupt_handler:
    iretq

%macro make_exception_handler 1
exception%1_handler:
    cli
    push byte 0
    push byte %1
    jmp exception_body
%endmacro

%macro make_error_exception_handler 1
exception%1_handler:
    cli
    push byte %1
    jmp exception_body
%endmacro

make_exception_handler 0
make_exception_handler 1
make_exception_handler 2
make_exception_handler 3
make_exception_handler 4
make_exception_handler 5
make_exception_handler 6
make_exception_handler 7
make_error_exception_handler 8
make_exception_handler 9
make_error_exception_handler 10
make_error_exception_handler 11
make_error_exception_handler 12
make_error_exception_handler 13
make_error_exception_handler 14
make_exception_handler 15
make_exception_handler 16
make_error_exception_handler 17
make_exception_handler 18
make_exception_handler 19

exception_handlers:
      dq exception0_handler
      dq exception1_handler
      dq exception2_handler
      dq exception3_handler
      dq exception4_handler
      dq exception5_handler
      dq exception6_handler
      dq exception7_handler
      dq exception8_handler
      dq exception9_handler
      dq exception10_handler
      dq exception11_handler
      dq exception12_handler
      dq exception13_handler
      dq exception14_handler
      dq exception15_handler
      dq exception16_handler
      dq exception17_handler
      dq exception18_handler
      dq exception19_handler
