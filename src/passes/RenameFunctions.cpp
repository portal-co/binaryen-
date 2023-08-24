#include "obfuscate.h"
#include "passes.h"
#include "pstream.h"
#include <memory>
using namespace wasm;
wasm::Pass* wasm::createRenameFunctionsPass() {
  std::shared_ptr<redi::pstream> s =
    std::make_shared<redi::pstream>(getenv("RENAMER"));
  std::shared_ptr<redi::pstream> sr =
    std::make_shared<redi::pstream>(getenv("REFERENCE_RENAMER"));
  std::shared_ptr<redi::pstream> se =
    std::make_shared<redi::pstream>(getenv("EXPORT_RENAMER"));
  std::shared_ptr<redi::pstream> si =
    std::make_shared<redi::pstream>(getenv("IMPORT_RENAMER"));
  return new SwizzlePass(
    [=](Name a, Pass* p, bool isRef) -> Name {
      redi::pstream* stra = &*s;
      if (isRef) {
        stra = &*sr;
      }
      redi::pstream& str = *stra;
      str << a.toString() << "\n";
      std::string s2;
      str >> s2;
      return s2;
    },
    [=](Importable* i, ExternalKind k, Pass* p) {
      redi::pstream& str = *si;
      str << i->module.toString() << " " << i->base.toString() << "\n";
      std::string s2, s3;
      str >> s2;
      str >> s3;
      i->module = s2;
      i->base = s3;
    },
    [=](Name a, ExternalKind k, Pass* p) -> Name {
      redi::pstream& str = *se;
      str << a.toString() << "\n";
      std::string s2;
      str >> s2;
      return s2;
    });
}