#pragma once

struct memory_base {
    int code_section_base_;
    int data_section_base_;
    int stack_top_;
    int interrupt_vector_base_;
};
