#include "ir/find_all.h"
#include "wasm-emscripten.h"
#include <string>
#include "support/string.h"
namespace wasm {
inline namespace code {
class StringConstantTracker {
public:
  StringConstantTracker(Module& wasm) : wasm(wasm) { calcSegmentOffsets(); }

  const char* stringAtAddr(Address address) {
    for (unsigned i = 0; i < wasm.dataSegments.size(); ++i) {
      auto& segment = wasm.dataSegments[i];
      Address offset = segmentOffsets[i];
      if (address >= offset && address < offset + segment->data.size()) {
        return &segment->data[address - offset];
      }
    }
    Fatal() << "unable to find data for ASM/EM_JS const at: " << address;
    return nullptr;
  }

  std::vector<Address> segmentOffsets; // segment index => address offset

private:
  void calcSegmentOffsets() {
    std::unordered_map<Name, Address> passiveOffsets;
    if (wasm.features.hasBulkMemory()) {
      // Fetch passive segment offsets out of memory.init instructions
      struct OffsetSearcher : PostWalker<OffsetSearcher> {
        std::unordered_map<Name, Address>& offsets;
        OffsetSearcher(std::unordered_map<Name, Address>& offsets)
          : offsets(offsets) {}
        void visitMemoryInit(MemoryInit* curr) {
          // The desitination of the memory.init is either a constant
          // or the result of an addition with __memory_base in the
          // case of PIC code.
          auto* dest = curr->dest->dynCast<Const>();
          if (!dest) {
            auto* add = curr->dest->dynCast<Binary>();
            if (!add) {
              return;
            }
            dest = add->left->dynCast<Const>();
            if (!dest) {
              return;
            }
          }
          auto it = offsets.find(curr->segment);
          if (it != offsets.end()) {
            Fatal() << "Cannot get offset of passive segment initialized "
                       "multiple times";
          }
          offsets[curr->segment] = dest->value.getInteger();
        }
      } searcher(passiveOffsets);
      searcher.walkModule(&wasm);
    }
    for (unsigned i = 0; i < wasm.dataSegments.size(); ++i) {
      auto& segment = wasm.dataSegments[i];
      if (segment->isPassive) {
        auto it = passiveOffsets.find(segment->name);
        if (it != passiveOffsets.end()) {
          segmentOffsets.push_back(it->second);
        } else {
          // This was a non-constant offset (perhaps TLS)
          //   segmentOffsets.push_back(UNKNOWN_OFFSET);
        }
      } else if (auto* addrConst = segment->offset->dynCast<Const>()) {
        auto address = addrConst->value.getUnsigned();
        segmentOffsets.push_back(address);
      } else {
        // TODO(sbc): Wasm shared libraries have data segments with non-const
        // offset.
        segmentOffsets.push_back(0);
      }
    }
  }

  Module& wasm;
};
struct EmJsWalker : public PostWalker<EmJsWalker> {
  std::string prefix;
  Module& wasm;

  StringConstantTracker stringTracker;

  std::map<std::string, std::string> codeByName;
  EmJsWalker(Module& _wasm, std::string _prefix)
    : prefix(_prefix), wasm(_wasm), stringTracker(_wasm) {}

  void visitExportA(Export* curr) {
    if (!curr->name.startsWith(wasm::IString(prefix))) {
      return;
    }

    Address address;
    if (curr->kind == ExternalKind::Global) {
      auto* global = wasm.getGlobal(curr->value);
      Const* const_ = global->init->cast<Const>();
      address = const_->value.getUnsigned();
    } else if (curr->kind == ExternalKind::Function) {
      auto* func = wasm.getFunction(curr->value);
      // An EM_JS has a single const in the body. Typically it is just returned,
      // but in unoptimized code it might be stored to a local and loaded from
      // there, and in relocatable code it might get added to __memory_base etc.
      FindAll<Const> consts(func->body);
      if (consts.list.size() != 1) {
        Fatal() << "Unexpected generated __em_js__ function body: "
                << curr->name;
      }
      auto* addrConst = consts.list[0];
      address = addrConst->value.getUnsigned();
    } else {
      return;
    }

    auto code = stringTracker.stringAtAddr(address);
    auto funcName = std::string(curr->name.toString().substr(
      prefix.size(), curr->name.size() - prefix.size()));
    codeByName[funcName] = code;
  }
  void visitExportB(Export* curr) {
    wasm::String::Split s(curr->name.toString(), "_");
    if (s.size() != 3) {
      return;
    }
    if (s[0] != wasm::String::wayEncode(prefix)) {
      return;
    }
    codeByName[s[1]] = wasm::String::wayDecode(s[2]);
  }
  void visitExport(Export* curr) {
    visitExportA(curr);
    visitExportB(curr);
  }
};

EmJsWalker findSpliceFuncsAndReturnWalker(Module& wasm, std::string prefix) {
  EmJsWalker walker(wasm, prefix);
  walker.walkModule(&wasm);
  return walker;
}
} // namespace code
} // namespace wasm