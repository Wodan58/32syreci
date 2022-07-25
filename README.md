32syreci
--------

A virtual machine from [Sympas](https://github.com/nickelsworth/sympas/blob/master/text/14-symbol-table-and-recursion.org), translated to C. The virtual
machine is a register machine.

Changes
-------

The text doesn't come with a Pascal version of 32syrecc, the compiler that
creates the file that 23syreci understands. But the text gives enough details.

Installation
------------

    make

Running
-------

    ./syrecc factorial.inp | ./dump
    ./syreci
