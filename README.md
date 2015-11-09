# Chalk Kernel

The goal is to create a library of components suitable for the
development of baremetal applications. Instead of relying on hardware
features for priviledge separation  it is our believe that _untrusted_
programs should be written in memory save languages or isolated
individually. This extends to kernel components such as device drivers.
Selected trusted programs could still be run without
memory protection with the only danger being that they might crash the
system.

More concretely this means we will aim to create
abstractions and drivers that enable the development of applications
which retain full control of scheduling on the CPUs, aswell as
physical memory allocation. The current plan is to make no use of the
virtual memory system and priviledge separation to enable isolated
processes, but instead use a combination of virtualization (VMX etc.), static
verification and memory safe languages to provide security.

In particular we will aim to create language abstractions nescesary to
write and specify device drivers and protocols in a memory safe and
efficient manner. The overall goal is to remove duplication of effort
and improve efficency of execution. Memory safe languages like Java,
C#, Haskell, Ocaml, Erlang, Smalltalk and Common Lisp at least
conceptually don't require most of the abstractions a monolythic
kernel provides.

In many cases maximum performance in current kernels can only be
achieved by bypassing several or all of the kernel abstractions of the
hardware, this is the case for example for network stacks, where user
space drivers can bring significant speed benifits (see Intels efforts
in this direction), aswell as Disk IO via NVME. Another prominent
example is the graphics stack where the most efficient way of using
the graphics hardware is to map the memory accessible by the GPU
directly into the memory space of the application bypassing the Kernel
for most of the work.