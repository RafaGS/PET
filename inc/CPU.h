#pragma once

#undef PC

// Macros de despacho de opcodes — prefijadas con CPU_ para evitar
// colisión con métodos de bitluni Graphics.h (A, C, D, E, O, U)
#define CPU_E(op, expr) case op: expr; break
#define CPU_O(op, fn)   CPU_E(op, fn())
#define CPU_A(op, e, a) CPU_E(op, e(a))
#define CPU_C(op)       case op:
#define CPU_D(fn)       default: fn(); break

// Mantener aliases sin prefijo para compatibilidad interna
// pero undefined antes de incluir cualquier header de display
#define E  CPU_E
#define O  CPU_O
#define A  CPU_A
#define C  CPU_C
#define D  CPU_D

#if defined(UNDOCUMENTED_OPS)
#define U(op, expr) case op: expr; break
#else
#define U(op, expr)
#endif

class CPU: public Checkpointable {
public:
    virtual ~CPU() {}
    virtual void run(unsigned instructions) = 0;
    virtual void reset() = 0;
    virtual char *status(char *buf, size_t n, bool hdr = false) = 0;

    virtual void checkpoint(Checkpoint &) = 0;
    virtual void restore(Checkpoint &) = 0;

    inline Memory::address pc() const { return PC; }
    inline bool halted() const { return _halted; }
    inline void halt()   { _halted = true; PC--; }
    inline void resume() { ++PC; _halted = false; }

    Memory &memory() const { return _mem; }

protected:
    CPU(Memory &mem): _mem(mem), _halted(false) {}
    Memory &_mem;
    Memory::address PC;
    bool _halted;
};
