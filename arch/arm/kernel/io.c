#include <linux/export.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/spinlock.h>

static DEFINE_RAW_SPINLOCK(__io_lock);

/*
 * Generic atomic MMIO modify.
 *
 * Allows thread-safe access to registers shared by unrelated subsystems.
 * The access is protected by a single MMIO-wide lock.
 */
void atomic_io_modify_relaxed(void __iomem *reg, u32 mask, u32 set)
{
	unsigned long flags;
	u32 value;

	raw_spin_lock_irqsave(&__io_lock, flags);
	value = readl_relaxed(reg) & ~mask;
	value |= (set & mask);
	writel_relaxed(value, reg);
	raw_spin_unlock_irqrestore(&__io_lock, flags);
}
EXPORT_SYMBOL(atomic_io_modify_relaxed);

void atomic_io_modify(void __iomem *reg, u32 mask, u32 set)
{
	unsigned long flags;
	u32 value;

	raw_spin_lock_irqsave(&__io_lock, flags);
	value = readl_relaxed(reg) & ~mask;
	value |= (set & mask);
	writel(value, reg);
	raw_spin_unlock_irqrestore(&__io_lock, flags);
}
EXPORT_SYMBOL(atomic_io_modify);

/*
 * Copy data from IO memory space to "real" memory space.
 */
void *asmcopy_8w(void *dst, void *src, int blocks);
asm("						\n\
	.align  2				\n\
	.text					\n\
	.global asmcopy_8w			\n\
	.type asmcopy_8w, %function		\n\
asmcopy_8w:					\n\
	stmfd sp!, {r3-r10, lr}			\n\
.loop:  ldmia r1!, {r3-r10}			\n\
	stmia r0!, {r3-r10}			\n\
	subs r2, r2, #1				\n\
	bne .loop				\n\
	ldmfd sp!, {r3-r10, pc}			\n\
");

void _memcpy_fromio(void *to, const volatile void __iomem *from, size_t count)
{
	unsigned char *dst = (unsigned char *)to;
	unsigned char *src = (unsigned char *)from;
	if ((((int)src & 3) == 0) && (((int)dst & 3) == 0) && (count >= 32)) {
		/* copy big chunks */
		asmcopy_8w(dst, src, count >> 5);
		dst += count & (~0x1f);
		src += count & (~0x1f);
		count &= 0x1f;
	}

	/* un-aligned or trailing accesses */
	while (count--) {
		*dst = readb(src);
		dst++;
		src++;
	}
}

/*
 * Copy data from "real" memory space to IO memory space.
 * This needs to be optimized.
 */
void _memcpy_toio(volatile void __iomem *to, const void *from, size_t count)
{
	const unsigned char *f = from;
	while (count) {
		count--;
		writeb(*f, to);
		f++;
		to++;
	}
}

/*
 * "memset" on IO memory space.
 * This needs to be optimized.
 */
void _memset_io(volatile void __iomem *dst, int c, size_t count)
{
	while (count) {
		count--;
		writeb(c, dst);
		dst++;
	}
}

EXPORT_SYMBOL(_memcpy_fromio);
EXPORT_SYMBOL(_memcpy_toio);
EXPORT_SYMBOL(_memset_io);
