## Inception

This is the artifact of Inception: Exposing New Attack Surfaces with Training in Transient Execution, [published](https://www.usenix.org/conference/usenixsecurity23/presentation/trujillo) at USENIX Security 2023. Inception is a novel transient execution attack that leaks arbitrary data on all AMD Zen CPUs, in the presence of all software- and hardware mitigations deployed to date. More information about Inception can be found [here](https://comsec.ethz.ch/inception).

## Organization

This artifact is split up in five parts.

* `tte_btb` determines whether the BTB can be manipulated in transient execution. To run this experiments, `arch.sh` should be configured.

* `tte_rsb` determines whether the RSB can be manipulated in transient execution. It may be necessary to run this experiment with the `-no-pie` CLANG argument.

* `phantomcall` shows that the RSB can be manipulated with a recursive PhantomCALL on all AMD Zen microarchitectures. It may be necessary to run this experiment with the `-no-pie` CLANG argument.

* `inception` leaks data of any arbitrary provided address on AMD Zen 1(+), Zen 2 and Zen 4 microarchitectures.

* `ibpb-eval` evaluates the overhead of using IBPB as a mitigation against Inception.
