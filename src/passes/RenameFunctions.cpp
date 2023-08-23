#include "obfuscate.h"
#include "passes.h"
#include "pstream.h"
#include <memory>
using namespace wasm;
wasm::Pass* wasm::createRenameFunctionsPass() {
    std::shared_ptr<redi::pstream> s = std::make_shared<redi::pstream>(getenv("RENAMER"));
  return new SwizzlePass(
    [=](Name a, Pass* p) -> Name {
      redi::pstream &str = *s;
      str << a.toString();
      std::string s2;
      str >> s2;
      return s2;
    },
    [=](Importable* i, ExternalKind k) {

    });
}