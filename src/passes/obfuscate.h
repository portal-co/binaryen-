#include "wasm.h"
#include <functional>
#include <map>
#include <memory>
#include <pass.h>
namespace wasm {
template<typename T> struct Types {
  T i32;
  T i64;
  T of(Type t) const {
    // switch (t) {
    //   case Type::I32:
    //     return i32;
    //   case Type::I64:
    //     return i64;
    // }
    if (t == Type::i32) {
      return i32;
    }
    if (t == Type::i64) {
      return i64;
    }
  }
};
inline std::map<BinaryOp, BinaryOp> invert_op() {
  return {
    {AddInt32, SubInt32},
    {SubInt32, AddInt32},
    {AddInt64, SubInt64},
    {SubInt64, AddInt64},
  };
};
struct Warp {
  std::function<Expression*(Expression*, Module*)> to;
  std::function<Expression*(Expression*, Module*)> from;
  inline Warp() {
    // Zero value
  }
  inline Warp(Warp first, Warp then)
    : to([=](Expression* e, Module* m) { return then.to(first.to(e, m), m); }),
      from([=](Expression* e, Module* m) {
        return first.from(then.from(e, m), m);
      }) {}
  inline Warp(Types<BinaryOp> c, Types<Expression*> f)
    : to([=](Expression* e, Module* m) {
        Binary* b = m->allocator.alloc<Binary>();
        b->op = c.of(e->type);
        b->left = e;
        b->right = f.of(e->type);
        return b;
      }),
      from([=](Expression* e, Module* m) {
        Binary* b = m->allocator.alloc<Binary>();
        b->op = invert_op()[c.of(e->type)];
        b->left = e;
        b->right = f.of(e->type);
        return b;
      }) {}
  inline Warp invert() {
    Warp w;
    w.from = to;
    w.to = from;
    return w;
  }
};
struct WarpLocals : WalkerPass<PostWalker<WarpLocals>> {
  Warp w;
  void visitLocalGet(LocalGet* curr) {
    replaceCurrent(w.from(curr, getModule()));
  }
  void visitLocalSet(LocalSet* curr) {
    curr->value = w.to(curr->value, getModule());
  }
  void visitCall(Call* c) {
    for (auto& t : c->operands)
      t = w.to(t, getModule());
  }
  void visitCallIndirect(CallIndirect* c) {
    for (auto& t : c->operands)
      t = w.to(t, getModule());
  }
};
template<typename S, typename I>
struct SwizzlePass : WalkerPass<PostWalker<SwizzlePass<S, I>>> {
  S swizzle;
  I swizzle_imports;
  SwizzlePass(S a, I b) : swizzle(a), swizzle_imports(b) {}
  void visitCall(Call* c) { c->target = swizzle(c->target, this); }
  void doWalkFunction(Function* func) {
    func->name = swizzle(func->name, this);
    if (func->imported()) {
      Importable* i = func;
      swizzle_imports(i, ExternalKind::Function);
    }
    this->walk(func->body);
  }
  void visitRefFunc(RefFunc* f) { f->func = swizzle(f->func, this); }
};
// inline auto rename(Name from, Name to) {
//   return SwizzlePass([=](Name a) -> Name {
//     if (a == from) {
//       return to;
//     }
//     if (a == to) {
//       return from;
//     }
//     return a;
//   },[=](Importable *i, ExternalKind k){

//   });
// }
struct ScratchSpacePass : Pass {
  virtual Name type() { WASM_UNREACHABLE("unimplemented"); }
  virtual Name transform(Name x) {
    return x.toString() + "$" + type().toString();
  }
  virtual void runOnFunction(Module* m, Function* f, Memory* scratch) {
    WASM_UNREACHABLE("unimplemented");
  }
  void run(Module* module) override {
    for (auto& fn : module->functions) {
      Memory* n = module->getMemoryOrNull(transform(fn->name));
      if (!n) {
        Memory m;
        m.setExplicitName(transform(fn->name));
        n = module->addMemory(std::make_unique<Memory>(std::move(m)));
      }
      runOnFunction(module, &*fn, n);
    }
  }
};

} // namespace wasm