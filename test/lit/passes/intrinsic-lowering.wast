;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --intrinsic-lowering -all -S -o - | filecheck %s

(module
  ;; CHECK:      (type $none (func))
  (type $none (func))

  ;; call.without.effects with no params.
  ;; CHECK:      (import "binaryen-intrinsics" "call.without.effects" (func $cwe-v (type $funcref_=>_i32) (param funcref) (result i32)))
  (import "binaryen-intrinsics" "call.without.effects" (func $cwe-v (param funcref) (result i32)))

  ;; call.without.effects with some params.
  ;; CHECK:      (import "binaryen-intrinsics" "call.without.effects" (func $cwe-dif (type $f64_i32_funcref_=>_f32) (param f64 i32 funcref) (result f32)))
  (import "binaryen-intrinsics" "call.without.effects" (func $cwe-dif (param f64) (param i32) (param funcref) (result f32)))

  ;; call.without.effects with no result.
  ;; CHECK:      (import "binaryen-intrinsics" "call.without.effects" (func $cwe-n (type $funcref_=>_none) (param funcref)))
  (import "binaryen-intrinsics" "call.without.effects" (func $cwe-n (param funcref)))

  ;; CHECK:      (func $test (type $ref?|$none|_=>_none) (param $none (ref null $none))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $make-i32)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (call $dif
  ;; CHECK-NEXT:    (f64.const 3.14159)
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call_ref $none
  ;; CHECK-NEXT:   (local.get $none)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test (param $none (ref null $none))
    ;; These will be lowered into calls.
    (drop (call $cwe-v (ref.func $make-i32)))
    (drop (call $cwe-dif (f64.const 3.14159) (i32.const 42) (ref.func $dif)))
    ;; The last must be a call_ref, as we don't see a constant ref.func
    (call $cwe-n (local.get $none))
  )

  ;; CHECK:      (func $make-i32 (type $none_=>_i32) (result i32)
  ;; CHECK-NEXT:  (i32.const 1)
  ;; CHECK-NEXT: )
  (func $make-i32 (result i32)
    (i32.const 1)
  )

  ;; CHECK:      (func $dif (type $f64_i32_=>_f32) (param $0 f64) (param $1 i32) (result f32)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $dif (param f64) (param i32) (result f32)
    ;; Helper function for the above.
    (unreachable)
  )
)
