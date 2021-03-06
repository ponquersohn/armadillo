#ifndef KERNFUNC_H
#define KERNFUNC_H

int kernfunc_init(void);

void armadillo_insn_init(struct insn *, const void *);
void armadillo_insn_get_length(struct insn *insn);
int armadillo_insn_rip_relative(struct insn *insn);

void *armadillo_malloc(unsigned long size);
void armadillo_malloc_free(void *buf);


#endif